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

void kernel_init()
{
    console_init();
    gdt_init();
    // task_init();
    interrupt_init();

    asm volatile("sti"); // 开中断，会不断触发时钟中断
    u32 counter = 0;
    while(true) {
        DEBUGK("looping in kernel init %d...\n", counter++);
        delay(100000000);
    }
    return;
}
