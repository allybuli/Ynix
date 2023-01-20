#include "../include/ynix/memory.h"
#include "../include/ynix/types.h"
#include "../include/ynix/ynix.h"
#include "../include/ynix/debug.h"
#include "../include/ynix/assert.h"
#include "../include/ynix/stdlib.h"
#include "../include/ynix/string.h"

#define LOGK(fmt, args...) DEBUGK(fmt, ##args)

// 内存区域类型
#define ZONE_VALID 1     // 可用内存区域
#define ZONE_RESERVED 2  // 不可用内存区域

// 获取地址位置 addr 所在内存页的页索引
#define IDX(addr) ((u32)addr >> 12)
#define ASSERT_PAGE(addr) assert((addr & 0xfff) == 0)

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

void memory_page_init(u32 magic, u32 addr) {
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

static u32 start_page = 0;
// 位图数组，每个物理页用一个字节（8位）来记录引用数量
static u8* memory_map;
static u32 memory_map_pages;

// 位图管理内存
void memory_map_init() {
    memory_map = (u8*)memory_base;

    memory_map_pages = div_round_up(total_pages, PAGE_SIZE);
    LOGK("Memory map page count %d\n", memory_map_pages);

    free_pages -= memory_map_pages;
    
    memset(memory_map, 0, memory_map_pages * PAGE_SIZE);

    
    start_page = IDX(memory_base) + memory_map_pages;
    for(size_t i = 0; i < start_page; i ++) {
        memory_map[i] = 1;
    }

    LOGK("Total pages %d, free pages %d\n", total_pages, free_pages);
}

static u32 get_page() {
    for(size_t i = start_page; i < total_pages; i++) {
        if(!memory_map[i]) {
            memory_map[i] = 1;
            assert(free_pages >= 1);
            free_pages --;
            u32 page = ((u32)i << 12);
            LOGK("Get page 0x%p\n", page);
            return page;
        }
    }
    panic("Out of memory!!!");
}

static void put_page(u32 addr) {
    ASSERT_PAGE(addr);
    u32 index = IDX(addr);
    assert(index >= start_page && index < total_pages);
    // 至少有一个引用
    assert(memory_map[index] >= 1);
    memory_map[index] --;
    if(!memory_map[index]) {
        free_pages ++;
    }
    assert(free_pages > 0 && free_pages < total_pages);
    LOGK("Put page 0x%p\n", addr);
}

void memory_init(u32 magic, u32 addr) {
    memory_page_init(magic, addr);
    memory_map_init();
}

void memory_test() {
    u32 pages[10];
    for(size_t i = 0; i < 10; i++) {
        pages[i] = get_page();
    }
    for(size_t i = 0; i < 10; i++) {
        put_page(pages[i]);
    }
}