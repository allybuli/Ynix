#include "../include/ynix/memory.h"
#include "../include/ynix/types.h"
#include "../include/ynix/ynix.h"
#include "../include/ynix/debug.h"
#include "../include/ynix/assert.h"
#include "../include/ynix/stdlib.h"
#include "../include/ynix/string.h"
#include "../include/ynix/bitmap.h"

#define LOGK(fmt, args...) DEBUGK(fmt, ##args)

// 内存区域类型
#define ZONE_VALID 1     // 可用内存区域
#define ZONE_RESERVED 2  // 不可用内存区域

// 获取地址位置 addr 所在内存页的页索引
#define IDX(addr) ((u32)addr >> 12)
// 获取 addr 的页目录表索引
#define DIDX(addr) (((u32)addr >> 22) & 0x3ff)
// 获取 addr 的页表索引
#define TIDX(addr) (((u32)addr >> 12) & 0x3ff)
#define PAGE(idx) ((u32)idx << 12)
#define ASSERT_PAGE(addr) assert((addr & 0xfff) == 0)

// 内核页目录索引
#define KERNEL_PAGE_DIR 0x1000
// 内核页表索引
static u32 KERNEL_PAGE_TABLE[] = {
    0x2000,
    0x3000,
};

#define KERNEL_MAP_BITS 0x4000

// 内核大小：8M
#define KERNEL_MEMORY_SIZE (sizeof(KERNEL_PAGE_TABLE) * 0x100000)

bitmap_t kernel_map;

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

    if(memory_size < KERNEL_MEMORY_SIZE) {
        panic("System memory is %dM too small, at least %dM needed\n", memory_size / MEMORY_BASE, KERNEL_MEMORY_SIZE / MEMORY_BASE);
    }
}

static u32 start_page = 0;
// 位图数组，每个物理页用一个字节（8位）来记录引用数量
static u8* memory_map;
static u32 memory_map_pages;

// 位图管理整个系统的内存
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

    u32 length = (IDX(KERNEL_MEMORY_SIZE) - IDX(MEMORY_BASE)) / 8;
    bitmap_init(&kernel_map, (char*)KERNEL_MAP_BITS, length, IDX(MEMORY_BASE));
    bitmap_scan(&kernel_map, memory_map_pages);
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

u32 get_cr3() {
    asm volatile("movl %cr3, %eax");
}

void set_cr3(u32 pde) {
    ASSERT_PAGE(pde);
    asm volatile(
        "movl %%eax, %%cr3\n"
        ::"a"(pde)
    );
}

static void enable_page() {
    asm volatile(
        "movl %cr0, %eax\n"
        "orl $0x80000000, %eax\n"
        "movl %eax, %cr0\n"
    );
}

// index 为物理页的索引
static void entry_init(page_entry_t* entry, u32 index) {
    *(u32*)entry = 0;
    entry->present = 1;
    entry->write = 1;
    entry->user = 1;
    entry->index = index;
}

void mapping_init() {
    // 页目录
    page_entry_t* pde = (page_entry_t*)KERNEL_PAGE_DIR;
    memset(pde, 0, PAGE_SIZE);

    // 最开始的8M内存分配给内核
    idx_t index = 0;
    assert(sizeof(KERNEL_PAGE_TABLE) > 0);
    for(idx_t idx = 0; idx < (sizeof(KERNEL_PAGE_TABLE) / sizeof(KERNEL_PAGE_TABLE[0])); idx ++) {
        // 一页页表
        page_entry_t* pte = (page_entry_t*)KERNEL_PAGE_TABLE[idx];
        memset(pte, 0, PAGE_SIZE);

        // 页目录表中的一个页表项
        page_entry_t* entry = &pde[idx];
        entry_init(entry, IDX((u32)pte));

        for(idx_t tidx = 0; tidx < ENTRY_SIZE; tidx ++, index ++) {
            // 第 0 页不映射，为造成空指针访问，缺页异常，便于排错
            if(0 == index) {
                continue;
            }
            // 页表中的一个页表项
            page_entry_t* entry = &pte[tidx];
            // assert(!memory_map[index]);
            entry_init(entry, index);
            memory_map[index] = 1;
        }
    }

    //将页目录的最后一个页表项指向页目录自己，开启分页后，可直接通过虚拟地址0xfffff000找到页目录，方便页目录及页表的修改
    page_entry_t* entry = &pde[ENTRY_SIZE-1];
    entry_init(entry, IDX(KERNEL_PAGE_DIR));
    // 为什么虚拟地址0xfffff000就是页目录的虚拟起始地址？
    // 0xfffff000
    // --> 0b1111_1111_11_11_1111_1111_0000_0000_0000
    // 1、页目录索引为0b1111_1111_11，也就是第1023项，就是最后一个页表项，这个页表项被设置为指向页目录这一页
    // 2、页表索引也为0b1111_1111_11，也就是页目录中的最后一个页表项，被预先设置为指向自身这一页
    // 3、页内偏移为0，也就是页目录这一页的开始位置

    // 两页内核页表的映射也是如此，虚拟地址0xffc00000就是页表的开始位置
    BMB;
    set_cr3((u32)pde);
    BMB;
    enable_page();
    BMB;
}

static page_entry_t* get_pde() {
    // 页目录表
    return (page_entry_t*)(0xfffff000);
}

static page_entry_t* get_pte(u32 vaddr) {
    // 页表
    return (page_entry_t*)(0xffc00000 | (DIDX(vaddr) << 12));
}

// 刷新虚拟地址 vaddr 的快表 TLB
static void flush_tlb(u32 vaddr) {
    asm volatile("invlpg (%0)" ::"r"(vaddr)
                : "memory");
}

void memory_init(u32 magic, u32 addr) {
    memory_page_init(magic, addr);
    memory_map_init();
    mapping_init();
}

static u32 scan_page(bitmap_t* map, u32 count) {
    assert(count > 0);
    int32 index = bitmap_scan(map, count);
    if(EOF == index) {
        panic("Scan page fail!!!");
    }

    u32 addr = PAGE(index);
    LOGK("Scan page 0x%p count %d\n", addr, count);
    return addr;
}

static void reset_page(bitmap_t* map, u32 addr, u32 count) {
    ASSERT_PAGE(addr);
    assert(count > 0);
    u32 index = IDX(addr);
    for(size_t i = 0; i < count; i++) {
        assert(bitmap_test(map, index + i));
        bitmap_set(map, index + i, false);
    }
}

u32 alloc_kpage(u32 count) {
    assert(count > 0);
    u32 addr = scan_page(&kernel_map, count);
    LOGK("Alloc kernel pages 0x%p count %d\n", addr, count);
    return addr;
}

void free_kpage(u32 addr, u32 count) {
    ASSERT_PAGE(addr);
    assert(count > 0);
    reset_page(&kernel_map, addr, count);
    LOGK("Free kernel pages 0x%p count %d\n", addr, count);
}

void memory_test() {
    u32* page = (u32*)(0x200000);
    u32 count = 0x6ff;
    for(size_t i = 0; i < count; i++) {
        page[i] = alloc_kpage(1);
        LOGK("0x%x\n", i);
    }
    for(size_t i = 0; i < count; i++) {
        free_kpage(page[i], 1);
    }
}