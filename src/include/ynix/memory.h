#ifndef YNIX_MEMORY_H
#define YNIX_MEMORY_H

#include "types.h"

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

// 一页的大小：4K
#define PAGE_SIZE 0x1000
// 一页内存里的页表项个数
#define ENTRY_SIZE (PAGE_SIZE / sizeof(page_entry_t))

// 可用内存起始位置：1M
#define MEMORY_BASE 0x100000

// 内核页目录索引
#define KERNEL_PAGE_DIR 0x1000

// 内核占用的内存大小 16M
#define KERNEL_MEMORY_SIZE 0x1000000

// 内核缓存地址
#define KERNEL_BUFFER_MEM 0x800000

// 内核缓存大小
#define KERNEL_BUFFER_SIZE 0x400000

// 内存虚拟磁盘地址
#define KERNEL_RAMDISK_MEM (KERNEL_BUFFER_MEM + KERNEL_BUFFER_SIZE)

// 内存虚拟磁盘大小
#define KERNEL_RAMDISK_SIZE 0x400000

// 用户栈顶地址 128M
#define USER_STACK_TOP 0x8000000

// 用户栈最大 2M
#define USER_STACK_SIZE 0x200000

// 用户栈底地址 128M - 2M
#define USER_STACK_BOTTOM (USER_STACK_TOP - USER_STACK_SIZE)

u32 get_cr2();
u32 get_cr3();
void set_cr3(u32 pde);

u32 alloc_kpage(u32 count);
void free_kpage(u32 addr, u32 count);

void link_page(u32 vaddr);
void unlink_page(u32 vaddr);

page_entry_t* copy_pde();
void free_pde();
int32 sys_brk(void*);

void memory_test();

#endif