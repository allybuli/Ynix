#include "../include/ynix/interrupt.h"
#include "../include/ynix/syscall.h"
#include "../include/ynix/debug.h"
#include "../include/ynix/mutex.h"
#include "../include/ynix/task.h"
#include "../include/ynix/stdio.h"
#include "../include/ynix/arena.h"

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

void test_recursion() {
    char tmp[0x400];
    test_recursion();
}

static void user_init_thread() {
    u32 counter = 0;
    while(true) {
    }
}

void init_thread() {
    char temp[100];
    task_to_user_mode(user_init_thread);
}

extern u32 keyboard_read(char *buf, u32 count);

void test_thread() {
    set_interrupt_state(true);
    test();
    while(true) {
        sleep(10);
    }
}