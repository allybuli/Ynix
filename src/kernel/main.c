#include "../include/ynix/ynix.h"
#include "../include/ynix/io.h"
#include "../include/ynix/types.h"
#include "../include/ynix/string.h"
#include "../include/ynix/console.h"
#include "../include/ynix/stdarg.h"
#include "../include/ynix/printk.h"

void kernel_init()
{
    console_init();

    int cnt = 30;
    while (cnt--)
    {
        printk("hello onix %#010x\n", cnt);
    }

    return;
}
