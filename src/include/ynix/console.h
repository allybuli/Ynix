#ifndef YXIN_CONSOLE_H
#define YXIN_CONSOLE_H

#include "types.h"

void console_clear();
void console_init();
void console_write(char *buf, u32 count);

#endif