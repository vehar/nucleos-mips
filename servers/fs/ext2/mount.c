/* Created (MFS based):
 *   February 2010 (Evgeniy Ivanov)
 */

#include "fs.h"
#include <nucleos/fcntl.h>
#include <nucleos/string.h>
#include <nucleos/com.h>
#include <nucleos/stat.h>
#include "buf.h"
#include "inode.h"
#include "super.h"
#include "drivers.h"
#include <servers/ds/ds.h>
#include <nucleos/vfsif.h>


/*===========================================================================*
 *				fs_readsuper				     *
 *===========================================================================*/
int fs_readsuper()
{
/* This function reads the superblock of the partition, gets the root inode
 * and sends back the details of them. Note, that the FS process does not
 * know the index of the vmnt object which refers to it, whenever the pathname
 * lookup leaves a partition an -ELEAVEMOUNT error is transferred back
 * so that the VFS knows that it has to find the vnode on which this FS
 * process' partition is mounted on.
 */
  struct inode *root_ip;
  cp_grant_id_t label_gid;
  size_t label_len;
  int r = 0;
  endpoint_t driver_e;
  int readonly, isroot;
  u32_t mask;

  fs_dev    = fs_m_in.REQ_DEV;
  label_gid = fs_m_in.REQ_GRANT;
  label_len = fs_m_in.REQ_PATH_LEN;
  readonly  = (fs_m_in.REQ_FLAGS & REQ_RDONLY) ? 1 : 0;
  isroot    = (fs_m_in.REQ_FLAGS & REQ_ISROOT) ? 1 : 0;

  if (label_len > sizeof(fs_dev_label))
	return(-EINVAL);

  r = sys_safecopyfrom(fs_m_in.m_source, label_gid, 0,
		       (vir_bytes)fs_dev_label, label_len, D);
  if (r != 0) {
	printk("%s:%d fs_readsuper: safecopyfrom failed: %d\n",
	       __FILE__, __LINE__, r);
	return(-EINVAL);
  }

  r= ds_retrieve_u32(fs_dev_label, &driver_e);
  if (r != 0)
  {
	printk("ext2:fs_readsuper: ds_retrieve_label_endpt failed for '%s': %d\n",
		fs_dev_label, r);
	return -EINVAL;
  }

  /* Map the driver endpoint for this major */
  driver_endpoints[(fs_dev >> MAJOR) & BYTE].driver_e =  driver_e;

  /* Open the device the file system lives on. */
  if (dev_open(driver_e, fs_dev, driver_e,
	readonly ? R_BIT : (R_BIT|W_BIT)) != 0) {
        return(-EINVAL);
  }

  /* Fill in the super block. */
  STATICINIT(superblock, sizeof(struct super_block));
  if (!superblock)
	panic("EXT2","Can't allocate memory for superblock.",NO_NUM);
  superblock->s_dev = fs_dev;	/* read_super() needs to know which dev */
  r = read_super(superblock);

  /* Is it recognized as a Minix filesystem? */
  if (r != 0) {
	superblock->s_dev = NO_DEV;
	dev_close(driver_e, fs_dev);
	return(r);
  }

  if (superblock->s_rev_level != EXT2_GOOD_OLD_REV) {
	struct super_block *sp = superblock; /* just shorter name */
	mask = ~SUPPORTED_INCOMPAT_FEATURES;
	if (HAS_INCOMPAT_FEATURE(sp, mask)) {
		if (HAS_INCOMPAT_FEATURE(sp, INCOMPAT_COMPRESSION & mask))
			printk("ext2: fs compression is not supported by server\n");
		if (HAS_INCOMPAT_FEATURE(sp, INCOMPAT_FILETYPE & mask))
			printk("ext2: fs in dir filetype is not supported by server\n");
		if (HAS_INCOMPAT_FEATURE(sp, INCOMPAT_RECOVER & mask))
			printk("ext2: fs recovery is not supported by server\n");
		if (HAS_INCOMPAT_FEATURE(sp, INCOMPAT_JOURNAL_DEV & mask))
			printk("ext2: fs journal dev is not supported by server\n");
		if (HAS_INCOMPAT_FEATURE(sp, INCOMPAT_META_BG & mask))
			printk("ext2: fs meta bg is not supported by server\n");
		return(-EINVAL);
	}
	mask = ~SUPPORTED_RO_COMPAT_FEATURES;
	if (HAS_RO_COMPAT_FEATURE(sp, mask)) {
		if (HAS_RO_COMPAT_FEATURE(sp, RO_COMPAT_SPARSE_SUPER & mask)) {
			printk("ext2: sparse super is not supported by server, \
				remount read-only\n");
		}
		if (HAS_RO_COMPAT_FEATURE(sp, RO_COMPAT_LARGE_FILE & mask)) {
			printk("ext2: large files are not supported by server, \
				remount read-only\n");
		}
		if (HAS_RO_COMPAT_FEATURE(sp, RO_COMPAT_BTREE_DIR & mask)) {
			printk("ext2: dir's btree is not supported by server, \
				remount read-only\n");
		}
		return(-EINVAL);
	}
  }

  if (superblock->s_state == EXT2_ERROR_FS) {
	printk("ext2: filesystem wasn't cleanly unmounted previous time\n");
        superblock->s_dev = NO_DEV;
	dev_close(driver_e, fs_dev);
	return(-EINVAL);
  }


  set_blocksize(superblock->s_block_size);

  /* Get the root inode of the mounted file system. */
  if ( (root_ip = get_inode(fs_dev, ROOT_INODE)) == NULL)  {
	printk("ext2: couldn't get root inode\n");
	  superblock->s_dev = NO_DEV;
	  dev_close(driver_e, fs_dev);
	  return(-EINVAL);
  }

  if (root_ip != NULL && root_ip->i_mode == 0) {
	  printk("%s:%d zero mode for root inode?\n", __FILE__, __LINE__);
	  put_inode(root_ip);
	  superblock->s_dev = NO_DEV;
	  dev_close(driver_e, fs_dev);
	  return(-EINVAL);
  }

  if (root_ip != NULL && (root_ip->i_mode & I_TYPE) != I_DIRECTORY) {
	printk("%s:%d root inode has wrong type, it's not a DIR\n",
		 __FILE__, __LINE__);
	put_inode(root_ip);
	superblock->s_dev = NO_DEV;
	dev_close(driver_e, fs_dev);
	return(-EINVAL);
  }

  superblock->s_rd_only = readonly;
  superblock->s_is_root = isroot;

  if (!readonly) {
	superblock->s_state = EXT2_ERROR_FS;
	superblock->s_mnt_count++;
	superblock->s_mtime = clock_time();
	write_super(superblock); /* Commit info, we just set above */
  }

  /* Root inode properties */
  fs_m_out.RES_INODE_NR = root_ip->i_num;
  fs_m_out.RES_MODE = root_ip->i_mode;
  fs_m_out.RES_FILE_SIZE_LO = root_ip->i_size;
  fs_m_out.RES_UID = root_ip->i_uid;
  fs_m_out.RES_GID = root_ip->i_gid;

  return(r);
}


/*===========================================================================*
 *				fs_mountpoint				     *
 *===========================================================================*/
int fs_mountpoint()
{
/* This function looks up the mount point, it checks the condition whether
 * the partition can be mounted on the inode or not.
 */
  register struct inode *rip;
  int r = 0;
  mode_t bits;

  /* Temporarily open the file. */
  if( (rip = get_inode(fs_dev, fs_m_in.REQ_INODE_NR)) == NULL)
	  return(-EINVAL);


  if(rip->i_mountpoint) r = -EBUSY;

  /* It may not be special. */
  bits = rip->i_mode & I_TYPE;
  if (bits == I_BLOCK_SPECIAL || bits == I_CHAR_SPECIAL) r = -ENOTDIR;

  put_inode(rip);

  if(r == 0) rip->i_mountpoint = TRUE;

  return(r);
}


/*===========================================================================*
 *				fs_unmount				     *
 *===========================================================================*/
int fs_unmount()
{
/* Unmount a file system by device number. */
  int count;
  struct inode *rip, *root_ip;

  if(superblock->s_dev != fs_dev) return(-EINVAL);

  /* See if the mounted device is busy.  Only 1 inode using it should be
   * open --the root inode-- and that inode only 1 time. */
  count = 0;
  for (rip = &inode[0]; rip < &inode[NR_INODES]; rip++)
	  if (rip->i_count > 0 && rip->i_dev == fs_dev) count += rip->i_count;

  if ((root_ip = find_inode(fs_dev, ROOT_INODE)) == NULL) {
	printk("ext2: couldn't find root inode. Unmount failed.\n");
	panic("EXT2","couldn't find root inode",NO_NUM);
	return(-EINVAL);
  }

  /* Sync fs data before checking count. In some cases VFS can force unmounting
   * and it will damage unsynced FS. We don't sync before checking root_ip since
   * if it is missing then something strange happened with FS, so it's better
   * to not use possibly corrupted data for syncing.
   */
  if (!superblock->s_rd_only) {
	/* force any cached blocks out of memory */
	(void) fs_sync();
  }

  if (count > 1) return(-EBUSY);	/* can't umount a busy file system */

  put_inode(root_ip);

  if (!superblock->s_rd_only) {
	superblock->s_wtime = clock_time();
	superblock->s_state = EXT2_VALID_FS;
	write_super(superblock); /* Commit info, we just set above */
  }

  /* Close the device the file system lives on. */
  dev_close(driver_endpoints[(fs_dev >> MAJOR) & BYTE].driver_e, fs_dev);

  /* Finish off the unmount. */
  superblock->s_dev = NO_DEV;
  unmountdone = TRUE;

  return(0);
}