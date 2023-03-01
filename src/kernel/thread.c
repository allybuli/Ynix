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
        // test();
        // test_recursion();
        printf("user mode %d %d %d\n", counter ++, getpid(), getppid());
        sleep(4000);
    }
}

void init_thread() {
    // set_interrupt_state(true);
    char temp[100];
    task_to_user_mode(user_init_thread);
}

void test_thread() {
    set_interrupt_state(true);
    u32 counter = 0;

    while(true) {
        LOGK("test task %d %d %d\n", counter++, getpid(), getppid());

        sleep(4000);
    }
}