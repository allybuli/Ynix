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

// 块位图 位大小(一字节8位)
#define BLOCK_BITS (BLOCK_SIZE * 8)

// 块 inode 数量
#define BLOCK_INODES (BLOCK_SIZE / sizeof(inode_desc_t)) 
// 块 dentry 数量
#define BLOCK_DENTRIES (BLOCK_SIZE / sizeof(dentry_t))
// 块索引数量
#define BLOCK_INDEXES (BLOCK_SIZE / sizeof(u16))

// 一个文件inode所能管理的文件块
// 直接块数量
#define DIRECT_BLOCK (7)
// 一级间接块数量
#define INDIRECT1_BLOCK BLOCK_INDEXES
// 二级间接块数量                  
#define INDIRECT2_BLOCK (INDIRECT1_BLOCK * INDIRECT1_BLOCK)
// 全部块数量
#define TOTAL_BLOCK (DIRECT_BLOCK + INDIRECT1_BLOCK + INDIRECT2_BLOCK)

// 目录分隔符 1
#define SEPARATOR1 '/'
// 目录分隔符 2
#define SEPARATOR2 '\\'
// 字符是否位目录分隔符
#define IS_SEPARATOR(c) (c == SEPARATOR1 || c == SEPARATOR2)

enum file_flag
{
    O_RDONLY = 00,      // 只读方式
    O_WRONLY = 01,      // 只写方式
    O_RDWR = 02,        // 读写方式
    O_ACCMODE = 03,     // 文件访问模式屏蔽码
    O_CREAT = 00100,    // 如果文件不存在就创建
    O_EXCL = 00200,     // 独占使用文件标志
    O_NOCTTY = 00400,   // 不分配控制终端
    O_TRUNC = 01000,    // 若文件已存在且是写操作，则长度截为 0
    O_APPEND = 02000,   // 以添加方式打开，文件指针置为文件尾
    O_NONBLOCK = 04000, // 非阻塞方式打开和操作文件
};

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
// {dev, nr} 唯一确定一个inode
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
    list_t inode_list;// 使用中 inode 链表
    inode_t* iroot;   // 根目录 inode
    inode_t* imount;  // 安装到的 inode
} super_block_t;

typedef struct dentry_t {
    u16 nr;           // 节点
    char name[14];    // 文件名
} dentry_t;

typedef struct file_t {
    inode_t *inode; // 文件 inode
    u32 count;      // 引用计数
    off_t offset;   // 文件偏移
    int flags;      // 文件标记
    int mode;       // 文件模式
} file_t;

super_block_t* get_super(dev_t dev);
super_block_t* read_super(dev_t dev);

// 分配一个文件块
idx_t balloc(dev_t dev);
// 释放一个文件块
void bfree(dev_t dev, idx_t idx);
// 分配一个文件系统 inode
idx_t ialloc(dev_t dev);
// 释放一个文件系统 inode
void ifree(dev_t dev, idx_t idx); 


// 获取 inode 第 block 块的索引值
// 如果不存在 且 create 为 true，则创建
idx_t bmap(inode_t *inode, idx_t block, bool create);

// 获取根目录 inode
inode_t *get_root_inode();
// 获得设备 dev 的 nr inode
inode_t *iget(dev_t dev, idx_t nr);
// 释放 inode
void iput(inode_t *inode);

// 获取 pathname 对应的父目录 inode
inode_t *named(char *pathname, char **next);
// 获取 pathname 对应的 inode 
inode_t *namei(char *pathname);

// 打开文件，返回 inode
inode_t *inode_open(char *pathname, int flag, int mode);

// 从 inode 的 offset 处，读 len 个字节到 buf
int inode_read(inode_t *inode, char *buf, u32 len, off_t offset);

// 从 inode 的 offset 处，将 buf 的 len 个字节写入磁盘
int inode_write(inode_t *inode, char *buf, u32 len, off_t offset);

// 释放 inode 所有文件块
void inode_truncate(inode_t *inode);

file_t *get_file();
void put_file(file_t *file);

#endif