#ifndef YNIX_ARENA_H
#define YNIX_ARENA_H

#include "types.h"
#include "list.h"

typedef list_node_t block_t;

typedef struct arena_descriptor_t {
    u32 total_block;  // 一页内存分成了多少块
    u32 block_size;   // 块的大小
    list_t free_list; // 空闲列表
} arena_descriptor_t;

typedef struct arena_t {
    arena_descriptor_t* desc;
    u32 free_count;  // 剩余空闲块数量
    u32 large;       // 该内存区域是否大于一页内存
    u32 magic;
} arena_t;

void* kmalloc(size_t);
void kfree(void*);

#endif