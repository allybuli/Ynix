#ifndef YNIX_MUTEX_H
#define YNIX_MUTEX_H

#include "types.h"
#include "list.h"

typedef struct mutex_t {
    bool value;
    list_t waiters;
} mutex_t;

void mutex_init(mutex_t*);
void mutex_lock(mutex_t*);
void mutex_unlock(mutex_t*);

// 可重入互斥锁
typedef struct lock_t {
    struct task_t* holder;
    mutex_t mutex;
    u32 repeat; // 重入次数
} lock_t;

void lock_init(lock_t*);
void lock_acquire(lock_t*);
void lock_release(lock_t*);

#endif