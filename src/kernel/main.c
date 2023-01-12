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

void kernel_init()
{
    console_init();
    gdt_init();
    task_init();
    return;
}
