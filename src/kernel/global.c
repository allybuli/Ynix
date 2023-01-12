#include "../include/ynix/global.h"
#include "../include/ynix/string.h"
#include "../include/ynix/debug.h"

// 内核全局描述符
descriptor_t gdt[GDT_SIZE];

// 内核全局描述符表指针
pointer_t gdt_ptr;

void gdt_init() {
    DEBUGK("init gdt!!!\n");
    asm volatile("sgdt gdt_ptr");

    memcpy(&gdt, (void*)gdt_ptr.base, gdt_ptr.limit + 1);

    gdt_ptr.base = (u32)&gdt;
    gdt_ptr.limit = sizeof(gdt) - 1;
    asm volatile("lgdt gdt_ptr\n");
}