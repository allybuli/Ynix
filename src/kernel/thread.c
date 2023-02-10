#include "../include/ynix/interrupt.h"
#include "../include/ynix/syscall.h"
#include "../include/ynix/debug.h"
#include "../include/ynix/mutex.h"

#define LOGK(fmt, args...) DEBUGK(fmt, ##args)

void idle_thread() {
    set_interrupt_state(true);
    while(true) {
        asm volatile(
            "sti\n"
            "hlt\n"
        );
        yield();       
    }
}

mutex_t mutex;

void init_thread() {
    mutex_init(&mutex);
    set_interrupt_state(true);
    u32 counter = 0;
    while(true) {
        mutex_lock(&mutex);
        LOGK("init task...\n", counter++);
        mutex_unlock(&mutex);
        // sleep(50000);
    }
}

void test_thread() {
    set_interrupt_state(true);
    u32 counter = 0;
    while(true) {
        mutex_lock(&mutex);
        LOGK("test task...\n", counter++);
        mutex_unlock(&mutex);
        // sleep(70900);
    }
}