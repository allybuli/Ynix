#include "../include/ynix/interrupt.h"
#include "../include/ynix/syscall.h"
#include "../include/ynix/debug.h"
#include "../include/ynix/mutex.h"
#include "../include/ynix/task.h"
#include "../include/ynix/stdio.h"
#include "../include/ynix/arena.h"
#include "../include/ynix/fs.h"

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
    char buf[256];
    fd_t fd;
    int len = 0;
    fd = open("/hello.txt", O_RDWR, 0755);
    len = read(fd, buf, sizeof(buf));
    printf("hello.txt content: %s length %d\n", buf, len);
    close(fd);

    fd = open("/world.txt", O_CREAT | O_RDWR, 0755);
    len = write(fd, buf, len);
    close(fd);
    while(true) {
        char ch;
        read(stdin, &ch, 1);
        write(stdout, &ch, 1);
        sleep(10);
    }
}

void init_thread() {
    char temp[100];
    task_to_user_mode(user_init_thread);
}

extern u32 keyboard_read(char *buf, u32 count);

void test_thread() {
    set_interrupt_state(true);
    while(true) {
        // test();
        sleep(10);
    }
}