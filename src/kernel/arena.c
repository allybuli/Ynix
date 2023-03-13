#include "../include/ynix/arena.h"
#include "../include/ynix/stdlib.h"
#include "../include/ynix/memory.h"
#include "../include/ynix/string.h"
#include "../include/ynix/ynix.h"
#include "../include/ynix/assert.h"

#define DESC_COUNT 7

static arena_descriptor_t descriptors[DESC_COUNT]; // 管理内存池

void arena_init() {
    u32 block_size = 16; // 最小内存块为16字节
    for(size_t i = 0; i < DESC_COUNT; i++) {
        arena_descriptor_t* desc = &descriptors[i];
        desc->block_size = block_size;
        desc->total_block = (PAGE_SIZE - sizeof(arena_t)) / block_size;
        list_init(&desc->free_list);
        block_size <<= 1;
    }
}

static void* get_arena_block(arena_t* arena, u32 idx) {
    assert(arena->desc->total_block > idx);
    void* addr = (void*)(arena + 1);
    u32 gap = arena->desc->block_size * idx;
    return addr + gap;
}

static arena_t* get_block_arena(block_t* block) {
    // 内核物理内存申请粒度为一页，4KB
    return (arena_t*)((u32)block & 0xfffff000);
}

void* kmalloc(size_t size) {
    arena_descriptor_t* desc = NULL;
    arena_t* arena = NULL;
    block_t* block = NULL;
    char* addr = NULL;

    if(size > 1024) {
        u32 asize = size + sizeof(arena_t);
        u32 count = div_round_up(asize, PAGE_SIZE);

        arena = (arena_t*)alloc_kpage(count);
        memset(arena, 0, count * PAGE_SIZE);
        arena->large = true;
        arena->free_count = count;
        arena->desc = NULL;
        arena->magic = YNIX_MAGIC;

        addr = (char*)((u32)arena + sizeof(arena_t));
        return addr;
    }

    // 从内存池中分配内存
    for(size_t i = 0; i < DESC_COUNT; i++) {
        desc = &descriptors[i];
        if(desc->block_size >= size) {
            break;
        }
    }

    assert(desc != NULL);

    if(list_empty(&desc->free_list)) {
        // 对应大小的内存池中，没有剩余可用的内存块
        arena = (arena_t*)alloc_kpage(1);
        memset(arena, 0, PAGE_SIZE);

        arena->desc = desc;
        arena->large = false;
        arena->free_count = desc->total_block;
        arena->magic = YNIX_MAGIC;
        for(size_t i = 0; i < arena->free_count; i++) {
            block = get_arena_block(arena, i);
            assert(!list_search(&arena->desc->free_list, block));
            list_push(&desc->free_list, block);
            assert(list_search(&arena->desc->free_list, block));
        }
    }

    block = list_pop(&desc->free_list);
    arena = get_block_arena(block);
    assert(arena->magic == YNIX_MAGIC && !arena->large);
    arena->free_count --;
    return block;
}

void kfree(void* ptr) {
    assert(ptr != NULL);

    arena_t* arena = get_block_arena((block_t*)ptr);

    assert(arena->large == 1 || arena->large == 0);
    assert(arena->magic == YNIX_MAGIC);

    if(arena->large) {
        free_kpage((u32)arena, arena->free_count);
        return;
    }

    list_push(&arena->desc->free_list, ptr);
    arena->free_count ++;

    if(arena->free_count == arena->desc->total_block) {
        // 从内存池中移除
        for(size_t i = 0; i < arena->free_count; i++) {
            block_t* block = get_arena_block(arena, i);
            assert(list_search(&arena->desc->free_list, block));
            list_remove(block);
            assert(!list_search(&arena->desc->free_list, block));
        }
        free_kpage((u32)arena, 1);
    }
}