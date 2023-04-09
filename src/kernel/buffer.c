#include "../include/ynix/buffer.h"
#include "../include/ynix/memory.h"
#include "../include/ynix/assert.h"
#include "../include/ynix/device.h"
#include "../include/ynix/task.h"
#include "../include/ynix/debug.h"

#define LOGK(fmt, args...) DEBUGK(fmt, ##args)

static buffer_t *buffer_start = (buffer_t *)KERNEL_BUFFER_MEM;
static u32 buffer_count = 0;

// buffer_t 在左，data在右，两方向中间生长
// 记录当前 buffer_t 结构体位置
static buffer_t *buffer_ptr = (buffer_t *)KERNEL_BUFFER_MEM;

// 记录当前数据缓冲区位置
// 预留一个空的block
static void *buffer_data = (void *)(KERNEL_BUFFER_MEM + KERNEL_BUFFER_SIZE - BLOCK_SIZE);

#define HASH_COUNT 31 // 设为素数

static list_t free_list;  // 缓冲链表，存放被释放的块
static list_t wait_list;  // 等待进程链表
static list_t hash_table[HASH_COUNT];  // 哈希表，存放缓冲块

u32 hash(dev_t dev, idx_t block) {
    return (dev ^ block) % HASH_COUNT;
}

// 将缓冲块放入哈希表
static void hash_locate(buffer_t* bf) {
    u32 idx = hash(bf->dev, bf->block);
    list_t* list = &hash_table[idx];
    assert(!list_search(list, &bf->hnode));
    list_push(list, &bf->hnode);
}

// 将缓冲块移出哈希表
static void hash_remove(buffer_t* bf) {
    u32 idx = hash(bf->dev, bf->block);
    list_t* list = &hash_table[idx];
    assert(list_search(list, &bf->hnode));
    list_remove(&bf->hnode);
}

static buffer_t* get_from_hash_table(dev_t dev, idx_t block) {
    u32 idx = hash(dev, block);
    list_t* list = &hash_table[idx];
    buffer_t* bf = NULL;
    for(list_node_t* node = list->head.next; node != &list->tail; node = node->next) {
        buffer_t* ptr = element_entry(buffer_t, hnode, node);
        if(ptr->dev == dev && ptr->block == block) {
            bf = ptr;
            break;
        }
    }
    if(!bf) {
        return NULL;
    }
    if(list_search(&free_list, &bf->rnode)) {
        list_remove(&bf->rnode);
    }
    return bf;
}

static buffer_t* get_new_buffer() {
    buffer_t* bf = NULL;
    if((u32)(buffer_ptr + sizeof(buffer_t)) < (u32)buffer_data) {
        bf = buffer_ptr;
        bf->data = buffer_data;
        bf->dev = EOF;
        bf->block = 0;
        bf->count = 0;
        bf->dirty = false;
        bf->valid = false;
        lock_init(&bf->lock);
        buffer_count ++;
        buffer_ptr ++;
        buffer_data -= BLOCK_SIZE;
        LOGK("buffer count %d\n", buffer_count);
    }
    return bf;
}

static buffer_t* get_free_buffer() {
    buffer_t* bf = NULL;
    while(true) {
        // 优先让内存中存在更多可用的缓冲块
        bf = get_new_buffer();
        if(bf) {
            return bf;
        }
        if(!list_empty(&free_list)) {
            bf = element_entry(buffer_t, rnode, list_popback(&free_list));
            hash_remove(bf);
            bf->valid = false;
            return bf;
        }
        // 无可用缓冲块，需等待其它进程释放缓冲
        task_block(running_task(), &wait_list, TASK_BLOCKED);
    }
}

// 获取缓冲
buffer_t *getblk(dev_t dev, idx_t block) {
    buffer_t* bf = get_from_hash_table(dev, block);
    if(bf) {
        bf->count ++;
        return bf;
    }
    bf = get_free_buffer();
    assert(0 == bf->count);
    assert(0 == bf->dirty);

    bf->count ++;
    bf->dev = dev;
    bf->block = block;
    hash_locate(bf);
    return bf;
}

// 读缓冲
buffer_t *bread(dev_t dev, idx_t block) {
    buffer_t* buffer = getblk(dev, block);
    assert(buffer);
    if(buffer->valid) {
        return buffer;
    }
    // 可能存在多个进程同时并发的读，需互斥
    lock_acquire(&buffer->lock);
    if(!buffer->valid) {
        device_request(buffer->dev, buffer->data, BLOCK_SECS, buffer->block * BLOCK_SECS, 0, REQ_READ);
        buffer->dirty = false;
        buffer->valid = true;        
    }
    lock_release(&buffer->lock);
    return buffer;
}

// 写缓冲
void bwrite(buffer_t *bf) {
    assert(bf);
    if(!bf->dirty) { // 是否脏
        return;
    }
    // 读写的最小单位是块，（两个扇区大小）
    device_request(bf->dev, bf->data, BLOCK_SECS, bf->block * BLOCK_SECS, 0, REQ_WRITE);
    bf->dirty = false;
    bf->valid = true;
}

// 释放缓冲
void brelse(buffer_t *bf) {
    if(!bf) {
        return;
    }
    if(bf->dirty) {
        bwrite(bf);
    }
    bf->count --;
    assert(bf->count >= 0);
    if(bf->count) {
        return;
    }
    assert(!bf->rnode.next);
    assert(!bf->rnode.prev);
    list_push(&free_list, &bf->rnode);
    if(!list_empty(&wait_list)) {
        task_t* task = element_entry(task_t, node, list_popback(&wait_list));
        task_unblock(task);
    }
}

void buffer_init() {
    LOGK("buffer_t size is %d\n", sizeof(buffer_t));
    list_init(&free_list);
    list_init(&wait_list);
    for(size_t i = 0; i < HASH_COUNT; i++) {
        list_init(&hash_table[i]);
    }
}