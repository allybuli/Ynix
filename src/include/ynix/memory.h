#ifndef YNIX_MEMORY_H
#define YNIX_MEMORY_H

#include "types.h"

// 一页的大小：4K
#define PAGE_SIZE 0x1000

// 可用内存起始位置：1M
#define MEMORY_BASE 0x100000

typedef struct page_entry_t {
    u8 present : 1; // 是否在内存中
    u8 write : 1;   // 0只读，1可读可写
    u8 user : 1;    // 1所有人， 0超级用户
    u8 pwt : 1;     // 1直写模式，0回写模式
    u8 pcd : 1;     // 是否禁止该页缓冲
    u8 accessed : 1;// 统计页使用的频率
    u8 dirty : 1;   // 是否为脏页
    u8 pat : 1;     // 页大小 (4K/4M)
    u8 global : 1;  // 全局
    u8 ignored : 3; // 未使用，保留
    u32 index : 20; // 页索引
} _packed page_entry_t;

u32 get_cr3();
void set_cr3(u32 pde);

void memory_test();

#endif