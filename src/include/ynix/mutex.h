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

// 可重入自旋锁
typedef struct spinlock_t {
    struct task_t* holder;
    mutex_t mutex;
    u32 repeat; // 重入次数
} spinlock_t;

void spin_init(spinlock_t*);
void spin_lock(spinlock_t*);
void spin_unlock(spinlock_t*);

#endif