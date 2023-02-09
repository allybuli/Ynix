#include "../include/ynix/interrupt.h"
#include "../include/ynix/syscall.h"
#include "../include/ynix/debug.h"

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

void init_thread() {
    set_interrupt_state(true);
    u32 counter = 0;
    while(true) {
        LOGK("init task...\n", counter++);
        sleep(50000);
    }
}

void test_thread() {
    set_interrupt_state(true);
    u32 counter = 0;
    while(true) {
        LOGK("test task...\n", counter++);
        sleep(70900);
    }
}