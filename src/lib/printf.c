#include "../include/ynix/stdio.h"
#include "../include/ynix/stdarg.h"
#include "../include/ynix/syscall.h"

static char buf[1024];

int printf(const char *fmt, ...) {
    va_list args;
    int i;

    va_start(args, fmt);

    i = vsprintf(buf, fmt, args);

    va_end(args);

    write(stdout, buf, i);

    return i;
}
