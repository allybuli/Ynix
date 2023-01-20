#include "../include/ynix/ynix.h"
#include "../include/ynix/io.h"
#include "../include/ynix/types.h"
#include "../include/ynix/string.h"
#include "../include/ynix/console.h"
#include "../include/ynix/stdarg.h"
#include "../include/ynix/printk.h"
#include "../include/ynix/assert.h"
#include "../include/ynix/debug.h"
#include "../include/ynix/global.h"
#include "../include/ynix/interrupt.h"
#include "../include/ynix/stdlib.h"
#include "../include/ynix/task.h"
#include "../include/ynix/memory.h"

void kernel_init()
{
    // console_init();
    gdt_init();
    interrupt_init();

    memory_test();
    // task_init();
    
    asm volatile("sti"); // 开中断，会不断触发时钟中断
    hang();
    // u32 counter = 0;
    // while(true) {
    //     DEBUGK("looping in kernel init %d...\n", counter++);
    //     delay(100000000);
    // }
    return;
}
