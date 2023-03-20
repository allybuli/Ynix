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
        // printf("user mode %d %d %d\n", counter ++, getpid(), getppid());
        // pid_t pid = fork();
        // if(pid) {
        //     printf("father thread %d %d %d\n", pid, getpid(), getppid());
        //     int status = 0;
        //     // sleep(1000);
        //     pid_t child = waitpid(pid, &status);
        //     printf("wait pid %d status %d %d\n", child, status, counter++);
        // } else {
        //     printf("child thread %d %d %d\n", pid, getpid(), getppid());
        //     sleep(1000);
        //     exit(0);
        // }
        // sleep(2000);
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
    LOGK("test finished of task %d\n", getpid());
    while(true) {
        sleep(10);
    }
}