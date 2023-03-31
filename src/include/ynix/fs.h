#ifndef YNIX_FS_H
#define YNIX_FS_H

#include "types.h"
#include "buffer.h"

#define BLOCK_SIZE 1024 // 块大小(两个扇区大小)
#define SECTOR_SIZE 512 // 扇区大小

#define MINIX1_MAGIC 0x137F // 文件系统魔数
#define NAME_LEN 14        // 文件名长度

#define IMAP_NR 8      // indoe位图最大块数
#define ZMAP_NR 8      // 块位图最大块数

// 磁盘上的数据结构
typedef struct inode_desc_t {
    u16 mode;    // 文件类型和属性（rwx位）
    u16 uid;     // 用户id（文件拥有者标识符）
    u32 size;    // 文件大小（字节）
    u32 mtime;   // 修改时间戳
    u8 gid;      // 组id（文件拥有者所在的组）
    u8 nlinks;   // 链接数（多少个文件目录项指向该节点）
    u16 zone[9]; // 文件逻辑块,直接(0-6),间接(7),双重间接(8)
} inode_desc_t;

// 内存中的数据结构
typedef struct inode_t {
    inode_desc_t* desc;
    struct buffer_t* buf;
    dev_t dev;   // 设备号
    idx_t nr;    // inode节点下标
    u32 count;   // 引用计数
    // 记录修改时间，创建时间的数据结构 todo
    list_node_t node;
    dev_t mount; // 安装设备号
} inode_t;

// 磁盘上的数据结构
typedef struct super_desc_t {
    u16 inodes;          // inode节点数
    u16 zones;           // 逻辑块数
    u16 imap_blocks;     // inode节点位图所占数据块数
    u16 zmap_blocks;     // 逻辑块位图所占数据块数
    u16 first_data_zone; // 第一个数据逻辑块号
    u16 log_zone_size;   // 一个逻辑块有几个数据块
    u32 max_size;        // 文件最大长度
    u16 magic;           // 文件系统魔数
} super_desc_t;

// 内存中的数据结构
typedef struct super_block_t {
    super_desc_t* desc;
    struct buffer_t* buf;
    struct buffer_t* imaps[IMAP_NR];
    struct buffer_t* zmaps[ZMAP_NR];
    dev_t dev;
    list_t inode_list;
    inode_t* iroot;
    inode_t* imount;
} super_block_t;

typedef struct dentry_t {
    u16 nr;           // 节点
    char name[14];    // 文件名
};

// super_block_t* get_super(dev_t dev);
super_block_t* read_super(dev_t dev);

#endif