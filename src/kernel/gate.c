#include "../include/ynix/types.h"
#include "../include/ynix/interrupt.h"
#include "../include/ynix/debug.h"
#include "../include/ynix/assert.h"

#define SYSCALL_SIZE 64
handler_t syscall_table[SYSCALL_SIZE];
extern gate_t idt[IDT_SIZE];
extern void syscall_handler();

void syscall_check(u32 nr) {
    if(SYSCALL_SIZE <= nr) {
        panic("syscall nr error!!!");
    }
}

static void sys_default() {
    panic("syscall not implemented!!!");
}

static u32 sys_test() {
    DEBUGK("syscall test...\n");
    return 255;
}

void syscall_init() {
    gate_t* gate = &idt[0x80];
    gate->offset0 = (u32)syscall_handler & 0xffff;
    gate->offset1 = ((u32)syscall_handler >> 16) & 0xffff;
    gate->selector = 1 << 3; // 代码段
    gate->reserved = 0;      // 保留不用
    gate->type = 0b1110;     // 中断门
    gate->segment = 0;       // 系统段
    gate->DPL = 3;           // 用户态
    gate->present = 1;       // 有效

    for(size_t i = 0; i < SYSCALL_SIZE; i++) {
        syscall_table[i] = sys_default;
    }
    syscall_table[0] = sys_test;
}