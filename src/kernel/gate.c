#include "../include/ynix/types.h"
#include "../include/ynix/interrupt.h"
#include "../include/ynix/debug.h"
#include "../include/ynix/assert.h"
#include "../include/ynix/syscall.h"
#include "../include/ynix/task.h"
#include "../include/ynix/console.h"
#include "../include/ynix/memory.h"
#include "../include/ynix/ide.h"
#include "../include/ynix/string.h"
#include "../include/ynix/device.h"

#define LOGK(fmt, args...) DEBUGK(fmt, ##args)

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


extern ide_ctrl_t controllers[2];

static u32 sys_test() {    
    char ch;
    device_t* device;
    // device = device_find(DEV_KEYBOARD, 0);
    // assert(device);
    // device_read(device->dev, &ch, 1, 0, 0);

    // device = device_find(DEV_CONSOLE, 0);
    // assert(device);
    // device_write(device->dev, &ch, 1, 0, 0);

    void* buf = (void*)alloc_kpage(1);
    device = device_find(DEV_IDE_PART, 0);
    assert(device);
    memset(buf, running_task()->pid, 512);    
    device_request(device->dev, buf, 1, running_task()->pid, 0, REQ_WRITE);
    free_kpage((u32)buf, 1);
    return 255;
}

extern int32 console_write();

static u32 sys_write(fd_t fd, char* buf, u32 len) {
    if(stdout == fd || stderr == fd) {
        return console_write(NULL, buf, len);
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
    syscall_table[SYS_NR_GETPID] = sys_getpid;
    syscall_table[SYS_NR_GETPPID] = sys_getppid;
    syscall_table[SYS_NR_FORK] = task_fork;
    syscall_table[SYS_NR_EXIT] = task_exit;
    syscall_table[SYS_NR_WAITPID] = task_waitpid;
}