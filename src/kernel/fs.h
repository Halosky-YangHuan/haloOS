// +------------------+
// |  File descriptor |
// +------------------+
// |  Pathname        |
// +------------------+
// |  Directory       |
// +------------------+
// |  Inode           |
// +------------------+
// |  Logging         |
// +------------------+
// |  Buffer cache    |
// +------------------+
// |  Disk            |
// +------------------+

// [boot block] [super block] [log block] [inode block] [bit map] [data block] ...
//      0             1       2   ~   31  32    ~   45     46     47

#include "type.h"
#include "spinlock.h"
#include "sleeplock.h"

#define BSIZE 1024

#define NDIRECT 12

#define PGSIZE 4096
#define PGSHIFT 12  // 页内的偏移量

#define T_DIR     1   // Directory
#define T_FILE    2   // File
#define T_DEVICE  3   // Device

#define FSMAGIC 0x10203040

struct buf {
    int vaild;
    int disk;
    uint dev;
    uint blockno;
    struct sleeplock sk;    
    uint refcnt;
    struct buf *prev;
    struct buf *next;
    uchar data[BSIZE];    
};

struct superblock {
  uint magic;        // Must be FSMAGIC
  uint size;         // Size of file system image (blocks)
  uint nblocks;      // Number of data blocks
  uint ninodes;      // Number of inodes.
  uint nlog;         // Number of log blocks
  uint logstart;     // Block number of first log block
  uint inodestart;   // Block number of first inode block
  uint bmapstart;    // Block number of first free map block
};

struct inode {
  uint dev;   // device number
  uint inum;  // inode number
  int ref;    // ref count
  struct sleeplock splock;
  int vaild;  // inode已从磁盘读取？

  // disk node
  short type;           
  short major;          
  short minor;         
  short nlink;      
  uint size;     
  uint addrs[NDIRECT+1]; 
}

// disk inode 
struct dinode {
  short type;           // File type
  short major;          // Major device number (T_DEVICE only)
  short minor;          // Minor device number (T_DEVICE only)
  short nlink;          // Number of links to inode in file system
  uint size;            // Size of file (bytes)
  uint addrs[NDIRECT+1];   // Data block addresses
};

struct file {
  enum { FD_NONE, FD_PIPE, FD_INODE, FD_DEVICE } type;
  int ref;            // reference count
  char readable;
  char writable;
  struct pipe *pipe; // FD_PIPE
  struct inode *ip;  // FD_INODE and FD_DEVICE
  uint off;          // FD_INODE
  short major;       // FD_DEVICE
}
