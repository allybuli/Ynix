#ifndef YNIX_BITMAP_H
#define TNIX_BITMAP_H

#include "types.h"

typedef struct bitmap_t {
    u8* bits;   // 位图缓冲区起始位置(内存最小的寻址单位为字节)
    u32 length; // 位图缓冲区长度
    u32 offset; // 位图在缓冲区的偏移
} bitmap_t;

void bitmap_init(bitmap_t* map, char* bits, u32 length, u32 offset);
void bitmap_make(bitmap_t* map, char* bits, u32 length, u32 offset);
bool bitmap_test(bitmap_t* map, u32 index);
void bitmap_set(bitmap_t* map, u32 index, bool value);
int bitmap_scan(bitmap_t* map, u32 count);

void bitmap_tests();

#endif