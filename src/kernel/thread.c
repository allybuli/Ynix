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

spinlock_t lock;

void init_thread() {
    spin_init(&lock);
    set_interrupt_state(true);
    u32 counter = 0;
    while(true) {
        spin_lock(&lock);
        LOGK("init task...\n", counter++);
        spin_unlock(&lock);
        // sleep(50000);
    }
}

void test_thread() {
    set_interrupt_state(true);
    u32 counter = 0;
    while(true) {
        spin_lock(&lock);
        LOGK("test task...\n", counter++);
        spin_unlock(&lock);
        // sleep(70900);
    }
}