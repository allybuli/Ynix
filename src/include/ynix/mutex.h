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

#endif