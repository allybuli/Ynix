#ifndef YNIX_STDLIB_H
#define YNIX_STDLIB_H

#include "types.h"

#define MAX(a, b) (a < b ? b : a)
#define MIN(a, b) (a < b ? a : b)

void delay(u32 count);
void hang();

u32 div_round_up(u32 num, u32 size);

int atoi(const char* str);

#endif