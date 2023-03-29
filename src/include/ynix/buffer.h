#ifndef YNIX_BUFFER_H
#define YNIX_BUFFER_H

#include "types.h"
#include "list.h"
#include "mutex.h"

#define BLOCK_SIZE 1024   // 缓存块大小（字节）
#define SECTOR_SIZE 512   // 扇区大小 （字节） 
#define BLOCK_SECS (BLOCK_SIZE / SECTOR_SIZE) // 一个块占两个扇区

typedef struct buffer_t {
    char* data;          // 数据区
    dev_t dev;           // 设备号
    idx_t block;         // 块号
    int count;           // 引用计数
    list_node_t hnode;   // 哈希表拉链节点
    list_node_t rnode;   // 缓冲节点
    lock_t lock;         // 锁
    bool dirty;          // 是否与磁盘不一致(脏)
    bool valid;          // 是否有效
} buffer_t;

buffer_t *getblk(dev_t dev, idx_t block);
buffer_t *bread(dev_t dev, idx_t block);
void bwrite(buffer_t *bf);
void brelse(buffer_t *bf);

#endif