#include "../include/ynix/bitmap.h"
#include "../include/ynix/string.h"
#include "../include/ynix/assert.h"

void bitmap_make(bitmap_t* map, char* bits, u32 length, u32 offset) {
    map->bits = bits;
    map->length = length;
    map->offset = offset;
}

void bitmap_init(bitmap_t* map, char* bits, u32 length, u32 offset) {
    memset(bits, 0, length);
    bitmap_make(map, bits, length, offset);
}

bool bitmap_test(bitmap_t* map, u32 index) {
    assert(index >= map->offset);

    idx_t idx = index - map->offset;
    u32 byte = idx / 8;
    u8 bit = idx % 8;
    assert(byte < map->length);
    return (map->bits[byte] & (1 << bit));
}

void bitmap_set(bitmap_t* map, u32 index, bool value) {
    assert(index >= map->offset);

    idx_t idx = index - map->offset;
    u32 byte = idx / 8;
    u8 bit = idx % 8;
    assert(byte < map->length);
    if(value) {
        map->bits[byte] |= (1 << bit);
    } else {
        map->bits[byte] &= ~(1 << bit);
    }
}

int bitmap_scan(bitmap_t* map, u32 count) {
    u32 bitmap_bit_size = map->length * 8;
    u32 cur = 0;
    u32 start = 0;
    while(cur < bitmap_bit_size) {
        start = cur;
        while(cur - start < count && cur < bitmap_bit_size && !bitmap_test(map, map->offset+cur)) {
            cur ++;
        }
        if(cur - start == count) {
            for(size_t i = 0; i < count; i++) {
                bitmap_set(map, map->offset+start+i, true);
            }
            return map->offset + start;
        }
        cur ++;
    }
    // 位图中没有剩余连续的 count 位
    return EOF;
}

#include "../include/ynix/debug.h"

#define LOGK(fmt, args...) DEBUGK(fmt, ##args)

#define LEN 2
u8 buf[LEN];
bitmap_t map;

void bitmap_tests()
{
    bitmap_init(&map, buf, LEN, 0);
    for (size_t i = 0; i < 33; i++)
    {
        idx_t idx = bitmap_scan(&map, 1);
        if (idx == EOF)
        {
            LOGK("TEST FINISH\n");
            break;
        }
        LOGK("%d\n", idx);
    }
}