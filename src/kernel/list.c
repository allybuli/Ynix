#include "../include/ynix/list.h"
#include "../include/ynix/assert.h"

void list_init(list_t* list) {
    list->head.prev = NULL;
    list->tail.next = NULL;
    list->head.next = &list->tail;
    list->tail.prev = &list->head;
}

void list_insert_before(list_node_t* anchor, list_node_t* node) {
    node->prev = anchor->prev;
    node->next = anchor;

    anchor->prev->next = node;
    anchor->prev = node;
}

void list_insert_after(list_node_t* anchor, list_node_t* node) {
    node->next = anchor->next;
    node->prev = anchor;

    anchor->next->prev = node;
    anchor->next = node;
}

void list_push(list_t* list, list_node_t* node) {
    assert(!list_search(list, node));
    list_insert_after(&list->head, node);    
}

list_node_t* list_pop(list_t* list) {
    assert(!list_empty(list));

    list_node_t *node = list->head.next;
    list_remove(node);

    return node;
}

void list_pushback(list_t* list, list_node_t* node) {
    assert(!list_search(list, node));
    list_insert_before(&list->tail, node);
}

list_node_t* list_popback(list_t* list) {
    assert(!list_empty(list));

    list_node_t *node = list->tail.prev;
    list_remove(node);

    return node;
}

bool list_search(list_t* list, list_node_t* node) {
    list_node_t* cur = list->head.next;
    while(cur != &list->tail) {
        if(cur == node) {
            return true;
        }
        cur = cur->next;
    }
    return false;
}

void list_remove(list_node_t* node) {
    assert(node->prev && node->next);
    node->prev->next = node->next;
    node->next->prev = node->prev;
    node->prev = NULL;
    node->next = NULL;
}

bool list_empty(list_t* list) {
    return list->head.next == &list->tail;
}

u32 list_size(list_t* list) {
    u32 size = 0;
    list_node_t* cur = list->head.next;
    while(cur != &list->tail) {
        size ++;
        cur = cur->next;
    }
    return size;
}

// 链表插入排序
void list_insert_sort(list_t *list, list_node_t *node, int offset)
{
    // 从链表找到第一个比当前节点 key 点更大的节点，进行插入到前面
    list_node_t *anchor = &list->tail;
    int key = element_node_key(node, offset);
    for (list_node_t *ptr = list->head.next; ptr != &list->tail; ptr = ptr->next)
    {
        int compare = element_node_key(ptr, offset);
        if (compare > key)
        {
            anchor = ptr;
            break;
        }
    }

    assert(node->next == NULL);
    assert(node->prev == NULL);

    // 插入链表
    list_insert_before(anchor, node);
}