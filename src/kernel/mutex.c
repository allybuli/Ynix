#include "../include/ynix/mutex.h"
#include "../include/ynix/interrupt.h"
#include "../include/ynix/task.h"
#include "../include/ynix/assert.h"
#include "../include/ynix/ynix.h"

void mutex_init(mutex_t* mutex) {
    mutex->value = false;
    list_init(&mutex->waiters);
}

void mutex_lock(mutex_t* mutex) {
    bool intr = interrupt_disable();

    task_t* cur = running_task();
    while(true == mutex->value)  {
        task_block(cur, &mutex->waiters, TASK_BLOCKED);
    }

    assert(false == mutex->value);
    
    mutex->value = true;
    assert(true == mutex->value);

    set_interrupt_state(intr);
}

void mutex_unlock(mutex_t* mutex) {
    bool intr = interrupt_disable();

    assert(true == mutex->value);

    mutex->value = false;
    assert(false == mutex->value);

    if(!list_empty(&mutex->waiters)) {
        task_t* task = element_entry(task_t, node, mutex->waiters.tail.prev);
        assert(YNIX_MAGIC == task->magic);
        task_unblock(task);

        // 保证新进程能获得互斥量，不然可能饿死
        task_yield();
    }

    set_interrupt_state(intr);
}

void lock_init(lock_t* lock) {
    lock->holder = NULL;
    lock->repeat = 0;
    mutex_init(&lock->mutex);
}

void lock_acquire(lock_t* lock) {
    task_t* cur = running_task();
    if(cur != lock->holder) {
        mutex_lock(&lock->mutex);
        lock->holder = cur;
        assert(lock->repeat == 0);
        lock->repeat = 1;
    } else {
        lock->repeat ++;
    }
}

void lock_release(lock_t* lock) {
    task_t* cur = running_task();
    assert(lock->holder == cur);
    if(lock->repeat > 1) {
        lock->repeat --;
        return;
    }

    assert(lock->repeat == 1);

    lock->holder = NULL;
    lock->repeat = 0;
    mutex_unlock(&lock->mutex);
}