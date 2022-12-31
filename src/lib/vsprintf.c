/* vsprintf.c -- Lars Wirzenius & Linus Torvalds. */
/*
 * Wirzenius wrote this portably, Torvalds fucked it up :-)
 */

#include "../include/ynix/stdio.h"

int vsprintf(char* buf, const char* fmt, va_list args) {
    
}

// 结果按格式输出字符串到 buf
int sprintf(char* buf, const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    int i = vsprintf(buf, fmt, args);
    va_end(args);
    return i;
}