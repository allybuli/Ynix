#include "../include/ynix/types.h"
#include "../include/ynix/interrupt.h"
#include "../include/ynix/debug.h"
#include "../include/ynix/assert.h"
#include "../include/ynix/syscall.h"
#include "../include/ynix/task.h"
#include "../include/ynix/console.h"
#include "../include/ynix/memory.h"

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
    char* addr = 0x1600000;
    link_page(addr);
    addr[3] = 'T';
    unlink_page(addr);

    return 255;
}

static u32 sys_write(fd_t fd, char* buf, u32 len) {
    if(stdout == fd || stderr == fd) {
        return console_write(buf, len);
    }
    panic("write!!!");
    return 0;
}

extern void task_yield();

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
    syscall_table[SYS_NR_TEST] = sys_test;
    syscall_table[SYS_NR_SLEEP] = task_sleep;
    syscall_table[SYS_NR_YIELD] = task_yield;
    syscall_table[SYS_NR_WRITE] = sys_write;
    syscall_table[SYS_NR_BRK] = sys_brk;
}