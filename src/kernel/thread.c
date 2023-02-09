#include "../include/ynix/interrupt.h"
#include "../include/ynix/syscall.h"
#include "../include/ynix/debug.h"

#define LOGK(fmt, args...) DEBUGK(fmt, ##args)

u32 idle_thread() {
    set_interrupt_state(true);
    u32 counter = 0;
    while(true) {
        LOGK("idle task.... %d\n", counter++);
        asm volatile(
            "sti\n"
            "hlt\n"
        );
        yield();       
    }
    return -1;    
}

u32 init_thread() {
    set_interrupt_state(true);
    while(true) {
        LOGK("init task...\n");
    }
    return -1;
}