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

lock_t lock;

void init_thread() {
    lock_init(&lock);
    set_interrupt_state(true);
    u32 counter = 0;
    while(true) {
        lock_acquire(&lock);
        LOGK("init task...\n", counter++);
        lock_release(&lock);
        // sleep(50000);
    }
}

void test_thread() {
    set_interrupt_state(true);
    u32 counter = 0;
    while(true) {
        lock_acquire(&lock);
        LOGK("test task...\n", counter++);
        lock_release(&lock);
        // sleep(70900);
    }
}