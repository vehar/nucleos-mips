/*
 *  Copyright (C) 2010  Ladislav Klenovic <klenovic@nucleonsoft.com>
 *
 *  This file is part of Nucleos kernel.
 *
 *  Nucleos kernel is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, version 2 of the License.
 */
/* Tables sizes */
#define V1_NR_DZONES       7 /* # direct zone numbers in a V1 inode */
#define V1_NR_TZONES       9 /* total # zone numbers in a V1 inode */
#define V2_NR_DZONES       7 /* # direct zone numbers in a V2 inode */
#define V2_NR_TZONES      10 /* total # zone numbers in a V2 inode */

#define NR_INODES        256 /* # slots in "in core" inode table */

#define INODE_HASH_LOG2   7     /* 2 based logarithm of the inode hash size */
#define INODE_HASH_SIZE   ((unsigned long)1<<INODE_HASH_LOG2)
#define INODE_HASH_MASK   (((unsigned long)1<<INODE_HASH_LOG2)-1)


/* The type of sizeof may be (unsigned) long.  Use the following macro for
 * taking the sizes of small objects so that there are no surprises like
 * (small) long constants being passed to routines expecting an int.
 */
#define usizeof(t) ((unsigned) sizeof(t))

/* File system types. */
#define SUPER_MAGIC   0x137F /* magic number contained in super-block */
#define SUPER_REV     0x7F13 /* magic # when 68000 disk read on PC or vv */
#define SUPER_V2      0x2468 /* magic # for V2 file systems */
#define SUPER_V2_REV  0x6824 /* V2 magic written on PC, read on 68K or vv */
#define SUPER_V3      0x4d5a /* magic # for V3 file systems */

#define V1   1 /* version number of V1 file systems */
#define V2   2 /* version number of V2 file systems */
#define V3   3 /* version number of V3 file systems */

/* Miscellaneous constants */
#define SU_UID       ((MNX(uid_t)) 0) /* super_user's uid_t */
#define SERVERS_UID ((MNX(uid_t)) 11) /* who may do FSSIGNON */
#define SYS_UID      ((MNX(uid_t)) 0) /* uid_t for processes MM and INIT */
#define SYS_GID      ((MNX(gid_t)) 0) /* gid_t for processes MM and INIT */
#define NORMAL       0  /* forces get_block to do disk read */
#define NO_READ      1  /* prevents get_block from doing disk read */
#define PREFETCH     2  /* tells get_block not to read or mark dev */

#define NO_BIT   ((MNX(bit_t)) 0) /* returned by alloc_bit() to signal failure */

#define LOOK_UP            0 /* tells search_dir to lookup string */
#define ENTER              1 /* tells search_dir to make dir entry */
#define DELETE             2 /* tells search_dir to delete entry */
#define IS_EMPTY           3 /* tells search_dir to ret. OK or ENOTEMPTY */  

/* write_map() args */
#define WMAP_FREE    (1 << 0)

#define PATH_TRANSPARENT 000   /* parse_path stops at final object */
#define PATH_PENULTIMATE 001   /* parse_path stops at last but one name */
#define PATH_OPAQUE      002   /* parse_path stops at final name */
#define PATH_NONSYMBOLIC 004   /* parse_path scans final name if symbolic */
#define PATH_STRIPDOT    010   /* parse_path strips /. from path */
#define EAT_PATH         PATH_TRANSPARENT
#define EAT_PATH_OPAQUE  PATH_OPAQUE
#define LAST_DIR         PATH_PENULTIMATE
#define LAST_DIR_NOTDOT  PATH_PENULTIMATE | PATH_STRIPDOT
#define LAST_DIR_EATSYM  PATH_NONSYMBOLIC

#define BYTE_SWAP          0 /* tells conv2/conv4 to swap bytes */

#define END_OF_FILE   (-104) /* eof detected */

#define ROOT_INODE        1  /* inode number for root directory */
#define BOOT_BLOCK        ((MNX(block_t)) 0) /* block number of boot block */
#define SUPER_BLOCK_BYTES (1024) /* bytes offset */
#define START_BLOCK       2      /* first block of FS (not counting SB) */

#define DIR_ENTRY_SIZE      usizeof (struct MNX(direct))  /* # bytes/dir entry   */
#define NR_DIR_ENTRIES(b)   ((b)/DIR_ENTRY_SIZE)  /* # dir entries/blk   */
#define SUPER_SIZE          usizeof (struct MNX(super_block))  /* super_block size    */
#define PIPE_SIZE(b)        (V1_NR_DZONES*(b))  /* pipe size in bytes  */

#define FS_BITMAP_CHUNKS(b)   ((b)/usizeof (MNX(bitchunk_t)))/* # map chunks/blk   */
#define FS_BITCHUNK_BITS      (usizeof(MNX(bitchunk_t)) * CHAR_BIT)
#define FS_BITS_PER_BLOCK(b)  (FS_BITMAP_CHUNKS(b) * FS_BITCHUNK_BITS)

/* Derived sizes pertaining to the V1 file system. */
#define V1_ZONE_NUM_SIZE   usizeof (MNX(zone1_t))  /* # bytes in V1 zone  */
#define V1_INODE_SIZE      usizeof (MNX(d1_inode))  /* bytes in V1 dsk ino */

/* # zones/indir block */
#define V1_INDIRECTS (_STATIC_BLOCK_SIZE/V1_ZONE_NUM_SIZE)

/* # V1 dsk inodes/blk */
#define V1_INODES_PER_BLOCK (_STATIC_BLOCK_SIZE/V1_INODE_SIZE)

/* Derived sizes pertaining to the V2 file system. */
#define V2_ZONE_NUM_SIZE     usizeof (MNX(zone_t))  /* # bytes in V2 zone  */
#define V2_INODE_SIZE        usizeof (MNX(d2_inode))  /* bytes in V2 dsk ino */
#define V2_INDIRECTS(b)   ((b)/V2_ZONE_NUM_SIZE)  /* # zones/indir block */
#define V2_INODES_PER_BLOCK(b) ((b)/V2_INODE_SIZE)/* # V2 dsk inodes/blk */

#define MFS_MIN(a,b) mfs_min_f(__FILE__,__LINE__,(a), (b))
#define MFS_NUL(str,l,m) mfs_nul_f(__FILE__,__LINE__,(str), (l), (m))

/* Args to dev_bio/dev_io */ 
#define MFS_DEV_READ    10001
#define MFS_DEV_WRITE   10002 
#define MFS_DEV_SCATTER 10003
#define MFS_DEV_GATHER  10004

