#include "../include/ynix/debug.h"
#include "../include/ynix/stdarg.h"
#include "../include/ynix/stdio.h"
#include "../include/ynix/printk.h"

static char buf[1024];

void debugk(char* file, int line, const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    vsprintf(buf, fmt, args);
    va_end(args);

    printk("[%s] [%d] %s", file, line, buf);
}