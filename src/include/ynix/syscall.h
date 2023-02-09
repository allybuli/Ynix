#ifndef YNIX_SYSCALL_H
#define YNIX_SYSCALL_H

#include "types.h"

typedef enum syscall_t {
    SYS_NR_TEST,
    SYS_NR_SLEEP,
    SYS_NR_YIELD,
} syscall_t;

u32 test();
void yield();
void sleep(u32 ms);

#endif