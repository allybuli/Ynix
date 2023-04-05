#include "../include/ynix/fs.h"
#include "../include/ynix/assert.h"
#include "../include/ynix/buffer.h"
#include "../include/ynix/bitmap.h"

// 分配一个文件块
idx_t balloc(dev_t dev) {
    super_block_t* sb = get_super(dev);
    assert(sb);

    buffer_t* buf = NULL;
    idx_t bit = EOF;
    bitmap_t map;

    for(size_t i = 0; i < ZMAP_NR; i++) {
        buf = sb->zmaps[i];
        assert(buf);

        // 将整个缓冲区作为位图
        bitmap_make(&map, buf->data, BLOCK_SIZE, sb->desc->first_data_zone + i * BLOCK_BITS - 1);

        bit = bitmap_scan(&map, 1);

        if(bit != EOF) {
            assert(bit < sb->desc->zones);
            buf->dirty = true;  // 设置为脏页
            break;
        }
    }
    bwrite(buf); // 强同步，任何修改都会进行磁盘和缓冲的数据同步
    return bit;
}

// 释放一个文件块
void bfree(dev_t dev, idx_t idx) {
    super_block_t* sb = get_super(dev);
    assert(sb);
    assert(idx < sb->desc->zones);

    size_t zmaps_idx = idx / BLOCK_BITS;
    buffer_t* buf = sb->zmaps[zmaps_idx];
    assert(buf);
    bitmap_t map;
    // 将整个缓冲区作为位图
    bitmap_make(&map, buf->data, BLOCK_SIZE, sb->desc->first_data_zone + BLOCK_BITS * zmaps_idx - 1);
    // 将 idx 对应的位图置位 0
    assert(bitmap_test(&map, idx));
    bitmap_set(&map, idx, 0);
    buf->dirty = true;  // 设置为脏页
    bwrite(buf);
} 

// 分配一个文件系统 inode
idx_t ialloc(dev_t dev) {
    super_block_t* sb = get_super(dev);
    assert(sb);

    buffer_t* buf = NULL;
    idx_t bit = EOF;
    bitmap_t map;

    for(size_t i = 0; i < IMAP_NR; i++) {
        buf = sb->imaps[i];
        assert(buf);

        bitmap_make(&map, buf->data, BLOCK_SIZE, i * BLOCK_BITS);
        bit = bitmap_scan(&map, 1);
        if(bit != EOF) {
            assert(bit < sb->desc->zones);
            buf->dirty = true;  // 设置为脏页
            break;
        }
    }
    bwrite(buf); // 强同步，任何修改都会进行磁盘和缓冲的数据同步
    return bit;
}   

// 释放一个文件系统 inode
void ifree(dev_t dev, idx_t idx) {
    super_block_t* sb = get_super(dev);
    assert(sb);
    assert(idx < sb->desc->zones);

    size_t imaps_idx = idx / BLOCK_BITS;
    buffer_t* buf = sb->imaps[imaps_idx];
    assert(buf);
    bitmap_t map;
    bitmap_make(&map, buf->data, BLOCK_SIZE, BLOCK_BITS * imaps_idx);
    assert(bitmap_test(&map, idx));
    bitmap_set(&map, idx, 0);
    buf->dirty = true;  // 设置为脏页
    bwrite(buf);
}