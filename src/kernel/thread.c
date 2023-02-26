#include "../include/ynix/interrupt.h"
#include "../include/ynix/syscall.h"
#include "../include/ynix/debug.h"
#include "../include/ynix/mutex.h"
#include "../include/ynix/task.h"
#include "../include/ynix/stdio.h"

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

static void real_init_thread() {
    u32 counter = 0;
    char ch;
    while(true) {
        // LOGK("init task...\n", counter++);
        // asm volatile("in $0x92, %ax\n");
        sleep(200);
        // LOGK("%c\n", ch);
        printf("user mode %d\n", counter ++);
    }
}

void init_thread() {
    // set_interrupt_state(true);
    char temp[100];
    task_to_user_mode(real_init_thread);
}

void test_thread() {
    set_interrupt_state(true);
    u32 counter = 0;
    while(true) {
        // LOGK("test task...\n", counter++);
        sleep(70900);
    }
}