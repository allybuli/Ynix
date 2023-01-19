#include "../include/ynix/memory.h"
#include "../include/ynix/types.h"
#include "../include/ynix/ynix.h"
#include "../include/ynix/debug.h"
#include "../include/ynix/assert.h"

#define LOGK(fmt, args...) DEBUGK(fmt, ##args)

// 内存区域类型
#define ZONE_VALID 1     // 可用内存区域
#define ZONE_RESERVED 2  // 不可用内存区域

// 获取地址位置 addr 所在内存页的页索引
#define IDX(addr) ((u32)addr >> 12)

// 内存区域
typedef struct ards_t {
    u64 base; // 基地址 
    u64 size; // 大小
    u32 type; // 类型（主要用来判断是否可用）
} _packed ards_t;

u32 memory_base = 0;
u32 memory_size = 0;
u32 total_pages = 0;
u32 free_pages = 0;

#define used_pages (total_pages - free_pages)

void memory_init(u32 magic, u32 addr) {
    u32 count = 0;
    ards_t* ptr = NULL;
    if(YNIX_MAGIC == magic) {
        count = *(u32*)addr;
        ptr = (ards_t*)(addr + 4);
        // 整个系统只使用最大的一块可用内存区域
        for(size_t i = 0; i < count; i++, ptr ++) {
            LOGK("Memory base 0x%p size %d type %d\n", (u32)ptr->base, (u32)ptr->size, ptr->type);
            if(ZONE_VALID == ptr->type && ptr->size > memory_size) {
                memory_base = ptr->base;
                memory_size = ptr->size;
            }
        }
    } else {
        panic("Memory init magic unknown 0x%p\n", magic);
    }

    LOGK("ARDS count %d\n", count);
    LOGK("Memory base 0x%p\n", (u32)memory_base);
    LOGK("Memory size 0x%p\n", (u32)memory_size);

    assert(MEMORY_BASE == memory_base);
    assert(0 == (memory_size & 0xfff));

    total_pages = IDX(memory_size) + IDX(memory_base);
    free_pages = IDX(memory_size);

    LOGK("Total pages %d\n", total_pages);
    LOGK("Free pages %d\n", free_pages);
}
