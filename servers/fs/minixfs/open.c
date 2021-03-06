/*
 *  Copyright (C) 2012  Ladislav Klenovic <klenovic@nucleonsoft.com>
 *
 *  This file is part of Nucleos kernel.
 *
 *  Nucleos kernel is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, version 2 of the License.
 */
#include <nucleos/kernel.h>
#include "fs.h"
#include <nucleos/stat.h>
#include <nucleos/fcntl.h>
#include <nucleos/string.h>
#include <nucleos/unistd.h>
#include <nucleos/com.h>
#include <servers/fs/minixfs/buf.h>
#include <servers/fs/minixfs/inode.h>
#include <servers/fs/minixfs/super.h>
#include <nucleos/vfsif.h>

static char mode_map[] = {R_BIT, W_BIT, R_BIT|W_BIT, 0};
static struct minix_inode *new_node(struct minix_inode *ldirp, 
	char *string, mode_t bits, zone_t z0);


/*===========================================================================*
 *				fs_create				     *
 *===========================================================================*/
int fs_create()
{
  phys_bytes len;
  int r, b;
  struct minix_inode *ldirp;
  struct minix_inode *rip;
  mode_t omode;
  char lastc[MINIXFS_NAME_MAX];
  
  /* Read request message */
  omode = fs_m_in.REQ_MODE;
  caller_uid = fs_m_in.REQ_UID;
  caller_gid = fs_m_in.REQ_GID;
  
  /* Try to make the file. */ 

  /* Copy the last component (i.e., file name) */
  len = MFS_MIN(fs_m_in.REQ_PATH_LEN, sizeof(lastc));
  err_code = sys_safecopyfrom(VFS_PROC_NR, fs_m_in.REQ_GRANT, 0,
			      (vir_bytes) lastc, (phys_bytes) len, D);
  if (err_code != 0) return err_code;
  MFS_NUL(lastc, len, sizeof(lastc));

  /* Get last directory inode (i.e., directory that will hold the new inode) */
  if ((ldirp = get_inode(fs_dev, fs_m_in.REQ_INODE_NR)) == NIL_INODE)
	  return(-ENOENT);

  /* Create a new inode by calling new_node(). */
  rip = new_node(ldirp, lastc, omode, NO_ZONE);
  r = err_code;

  /* If an error occurred, release inode. */
  if (r != 0) {
	  put_inode(ldirp);
	  put_inode(rip);
	  return(r);
  }
  
  /* Reply message */
  fs_m_out.m_source = rip->i_dev;  /* filled with FS endpoint by the system */
  fs_m_out.RES_INODE_NR = rip->i_num;
  fs_m_out.RES_MODE = rip->i_mode;
  fs_m_out.RES_FILE_SIZE_LO = rip->i_size;

  /* This values are needed for the execution */
  fs_m_out.RES_UID = rip->i_uid;
  fs_m_out.RES_GID = rip->i_gid;

  /* Drop parent dir */
  put_inode(ldirp);
  
  return(0);
}


/*===========================================================================*
 *				fs_mknod				     *
 *===========================================================================*/
int fs_mknod()
{
  struct minix_inode *ip, *ldirp;
  char lastc[MINIXFS_NAME_MAX];
  phys_bytes len;

  /* Copy the last component and set up caller's user and group id */
  len = MFS_MIN(fs_m_in.REQ_PATH_LEN, sizeof(lastc));
  err_code = sys_safecopyfrom(VFS_PROC_NR, fs_m_in.REQ_GRANT, 0,
			      (vir_bytes) lastc, (phys_bytes) len, D);
  if (err_code != 0) return err_code;
  MFS_NUL(lastc, len, sizeof(lastc));
  
  caller_uid = fs_m_in.REQ_UID;
  caller_gid = fs_m_in.REQ_GID;
  
  /* Get last directory inode */
  if((ldirp = get_inode(fs_dev, fs_m_in.REQ_INODE_NR)) == NIL_INODE)
	  return(-ENOENT);
  
  /* Try to create the new node */
  ip = new_node(ldirp, lastc, fs_m_in.REQ_MODE, (zone_t) fs_m_in.REQ_DEV);

  put_inode(ip);
  put_inode(ldirp);
  return(err_code);
}


/*===========================================================================*
 *				fs_mkdir				     *
 *===========================================================================*/
int fs_mkdir()
{
  int r1, r2;			/* status codes */
  ino_t dot, dotdot;		/* inode numbers for . and .. */
  struct minix_inode *rip, *ldirp;
  char lastc[MINIXFS_NAME_MAX];         /* last component */
  phys_bytes len;

  /* Copy the last component and set up caller's user and group id */
  len = MFS_MIN(fs_m_in.REQ_PATH_LEN, sizeof(lastc));
  err_code = sys_safecopyfrom(VFS_PROC_NR, fs_m_in.REQ_GRANT, 0,
			      (vir_bytes) lastc, (phys_bytes) len, D);
  if(err_code != 0) return(err_code);
  MFS_NUL(lastc, len, sizeof(lastc));

  caller_uid = fs_m_in.REQ_UID;
  caller_gid = fs_m_in.REQ_GID;
  
  /* Get last directory inode */
  if((ldirp = get_inode(fs_dev, fs_m_in.REQ_INODE_NR)) == NIL_INODE)
      return(-ENOENT);
  
  /* Next make the inode. If that fails, return error code. */
  rip = new_node(ldirp, lastc, fs_m_in.REQ_MODE, (zone_t) 0);
  
  if(rip == NIL_INODE || err_code == -EEXIST) {
	  put_inode(rip);		/* can't make dir: it already exists */
	  put_inode(ldirp);
	  return(err_code);
  }
  
  /* Get the inode numbers for . and .. to enter in the directory. */
  dotdot = ldirp->i_num;	/* parent's inode number */
  dot = rip->i_num;		/* inode number of the new dir itself */

  /* Now make dir entries for . and .. unless the disk is completely full. */
  /* Use dot1 and dot2, so the mode of the directory isn't important. */
  rip->i_mode = fs_m_in.REQ_MODE;	/* set mode */
  r1 = search_dir(rip, dot1, &dot, ENTER, IGN_PERM);/* enter . in the new dir*/
  r2 = search_dir(rip, dot2, &dotdot, ENTER, IGN_PERM); /* enter .. in the new
							 dir */

  /* If both . and .. were successfully entered, increment the link counts. */
  if (r1 == 0 && r2 == 0) {
	  /* Normal case.  It was possible to enter . and .. in the new dir. */
	  rip->i_nlinks++;	/* this accounts for . */
	  ldirp->i_nlinks++;	/* this accounts for .. */
	  ldirp->i_dirt = DIRTY;	/* mark parent's inode as dirty */
  } else {
	  /* It was not possible to enter . or .. probably disk was full -
	   * links counts haven't been touched. */
	  if(search_dir(ldirp, lastc, (ino_t *) 0, DELETE, IGN_PERM) != 0)
		  panic(__FILE__, "Dir disappeared ", rip->i_num);
	  rip->i_nlinks--;	/* undo the increment done in new_node() */
  }
  rip->i_dirt = DIRTY;		/* either way, i_nlinks has changed */

  put_inode(ldirp);		/* return the inode of the parent dir */
  put_inode(rip);		/* return the inode of the newly made dir */
  return(err_code);		/* new_node() always sets 'err_code' */
}


/*===========================================================================*
 *                             fs_slink 				     *
 *===========================================================================*/
int fs_slink()
{
  phys_bytes len;
  struct minix_inode *sip;           /* inode containing symbolic link */
  struct minix_inode *ldirp;         /* directory containing link */
  register int r;              /* error code */
  char string[MINIXFS_NAME_MAX];       /* last component of the new dir's path name */
  struct buf *bp;              /* disk buffer for link */
    
  caller_uid = fs_m_in.REQ_UID;
  caller_gid = fs_m_in.REQ_GID;
  
  /* Copy the link name's last component */
  len = MFS_MIN(fs_m_in.REQ_PATH_LEN, sizeof(string));
  r = sys_safecopyfrom(VFS_PROC_NR, fs_m_in.REQ_GRANT, 0,
		       (vir_bytes) string, (phys_bytes) len, D);
  if (r != 0) return(r);
  MFS_NUL(string, len, sizeof(string));
  
  /* Temporarily open the dir. */
  if( (ldirp = get_inode(fs_dev, fs_m_in.REQ_INODE_NR)) == NIL_INODE)
	  return(-EINVAL);
  
  /* Create the inode for the symlink. */
  sip = new_node(ldirp, string, (mode_t) (I_SYMBOLIC_LINK | RWX_MODES),
		   (zone_t) 0);

  /* Allocate a disk block for the contents of the symlink.
   * Copy contents of symlink (the name pointed to) into first disk block. */
  if( (r = err_code) == 0) {
	  r = (bp = new_block(sip, (off_t) 0)) == NIL_BUF ? err_code : 
		  sys_safecopyfrom(VFS_PROC_NR, fs_m_in.REQ_GRANT3, 0,
				   (vir_bytes) bp->b_data,
				   (vir_bytes) fs_m_in.REQ_MEM_SIZE, D);

	  if(r == 0) {
		  bp->b_data[_MIN_BLOCK_SIZE-1] = '\0';
		  sip->i_size = strlen(bp->b_data);
		  if(sip->i_size != fs_m_in.REQ_MEM_SIZE) {
			  /* This can happen if the user provides a buffer
			   * with a \0 in it. This can cause a lot of trouble
			   * when the symlink is used later. We could just use
			   * the strlen() value, but we want to let the user
			   * know he did something wrong. -ENAMETOOLONG doesn't
			   * exactly describe the error, but there is no
			   * -ENAMETOOWRONG.
			   */
			  r = -ENAMETOOLONG;
		  }
	  }
	  
	  put_block(bp, DIRECTORY_BLOCK); /* put_block() accepts NIL_BUF. */
  
	  if(r != 0) {
		  sip->i_nlinks = 0;
		  if(search_dir(ldirp, string, (ino_t *) 0, DELETE, 
							IGN_PERM) != 0)
					
			  panic(__FILE__, "Symbolic link vanished", NO_NUM);
	  } 
  }

  /* put_inode() accepts NIL_INODE as a noop, so the below are safe. */
  put_inode(sip);
  put_inode(ldirp);
  
  return(r);
}


/*===========================================================================*
 *				fs_newnode				     *
 *===========================================================================*/
int fs_newnode()
{
  register int r;
  mode_t bits;
  struct minix_inode *rip;

  caller_uid = fs_m_in.REQ_UID;
  caller_gid = fs_m_in.REQ_GID;
  bits = fs_m_in.REQ_MODE;

  /* Try to allocate the inode */
  if( (rip = alloc_inode(fs_dev, bits) ) == NIL_INODE)
	  return err_code;

  switch (bits & S_IFMT) {
	case S_IFBLK:
	case S_IFCHR:
		rip->i_zone[0] = fs_m_in.REQ_DEV; /* major/minor dev numbers*/
		break;
  }
  
  rw_inode(rip, WRITING);	/* mark inode as allocated */
  rip->i_update = ATIME | CTIME | MTIME;
  
  /* Fill in the fields of the response message */
  fs_m_out.RES_INODE_NR = rip->i_num;
  fs_m_out.RES_MODE = rip->i_mode;
  fs_m_out.RES_FILE_SIZE_LO = rip->i_size;
  fs_m_out.RES_UID = rip->i_uid;
  fs_m_out.RES_GID = rip->i_gid;
  fs_m_out.RES_DEV = (dev_t) rip->i_zone[0];

  return(0);
}


/*===========================================================================*
 *				new_node				     *
 *===========================================================================*/
static struct minix_inode *new_node(struct minix_inode *ldirp,
	char *string, mode_t bits, zone_t z0)
{
/* New_node() is called by fs_open(), fs_mknod(), and fs_mkdir().  
 * In all cases it allocates a new inode, makes a directory entry for it in
 * the ldirp directory with string name, and initializes it.  
 * It returns a pointer to the inode if it can do this; 
 * otherwise it returns NIL_INODE.  It always sets 'err_code'
 * to an appropriate value (0 or an error code).
 * 
 * The parsed path rest is returned in 'parsed' if parsed is nonzero. It
 * has to hold at least MINIXFS_NAME_MAX bytes.
 */

  register struct minix_inode *rip;
  register int r;

  /* Get final component of the path. */
  rip = advance(ldirp, string, IGN_PERM);

  if (S_ISDIR(bits) && 
      (ldirp)->i_nlinks >= ((ldirp)->i_sp->s_version == 1 ?
      CHAR_MAX : LINK_MAX)) {
        /* New entry is a directory, alas we can't give it a ".." */
        put_inode(rip);
        err_code = -EMLINK;
        return(NIL_INODE);
  }

  if ( rip == NIL_INODE && err_code == -ENOENT) {
	/* Last path component does not exist.  Make new directory entry. */
	if ( (rip = alloc_inode((ldirp)->i_dev, bits)) == NIL_INODE) {
		/* Can't creat new inode: out of inodes. */
		return(NIL_INODE);
	}

	/* Force inode to the disk before making directory entry to make
	 * the system more robust in the face of a crash: an inode with
	 * no directory entry is much better than the opposite.
	 */
	rip->i_nlinks++;
	rip->i_zone[0] = z0;		/* major/minor device numbers */
	rw_inode(rip, WRITING);		/* force inode to disk now */

	/* New inode acquired.  Try to make directory entry. */
	if((r=search_dir(ldirp, string, &rip->i_num, ENTER, IGN_PERM)) != 0) {
		rip->i_nlinks--;	/* pity, have to free disk inode */
		rip->i_dirt = DIRTY;	/* dirty inodes are written out */
		put_inode(rip);	/* this call frees the inode */
		err_code = r;
		return(NIL_INODE);
	}

  } else if (err_code == -EENTERMOUNT || err_code == -ELEAVEMOUNT) {
  	r = -EEXIST;
  } else { 
	/* Either last component exists, or there is some problem. */
	if (rip != NIL_INODE)
		r = -EEXIST;
	else
		r = err_code;
  }

  /* The caller has to return the directory inode (*ldirp).  */
  err_code = r;
  return(rip);
}


/*===========================================================================*
 *				fs_inhibread				     *
 *===========================================================================*/
int fs_inhibread()
{
  struct minix_inode *rip;
  
  if((rip = find_inode(fs_dev, fs_m_in.REQ_INODE_NR)) == NIL_INODE)
	  return(-EINVAL);

  /* inhibit read ahead */
  rip->i_seek = ISEEK;	
  
  return(0);
}

