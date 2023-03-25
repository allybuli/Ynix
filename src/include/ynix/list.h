#ifndef YNIX_LIST_H
#define YNIX_LIST_H

#include "types.h"

#define element_offset(type, member) (u32)(&((type *)0)->member)
#define element_entry(type, member, ptr) (type *)((u32)ptr - element_offset(type, member))
#define element_node_offset(type, node, key) ((int)(&((type *)0)->key) - (int)(&((type *)0)->node))
#define element_node_key(node, offset) *(int *)((int)node + offset)

// 侵入式链表
typedef struct list_node_t {
    struct list_node_t* prev;
    struct list_node_t* next;
} list_node_t;

typedef struct list_t {
    list_node_t head;
    list_node_t tail;
} list_t;

void list_init(list_t* list);
void list_insert_before(list_node_t* anchor, list_node_t* node);
void list_insert_after(list_node_t* anchor, list_node_t* node);
void list_push(list_t* list, list_node_t* node);
list_node_t* list_pop(list_t* list);
void list_pushback(list_t* list, list_node_t* node);
list_node_t* list_popback(list_t* list);
bool list_search(list_t* list, list_node_t* node);
void list_remove(list_node_t* node);
bool list_empty(list_t* list);
u32 list_size(list_t* list);

// 链表插入排序
void list_insert_sort(list_t *list, list_node_t *node, int offset);

#endif