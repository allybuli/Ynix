#include "../include/ynix/task.h"
#include "../include/ynix/printk.h"
#include "../include/ynix/debug.h"
#include "../include/ynix/stdlib.h"

#define STACK_SIZE 0x1000

task_t* a = (task_t *)0x1000;
task_t* b = (task_t *)0x2000;

extern void task_switch(task_t* next);

task_t* running_task() {
    // 返回栈帧的栈顶指针，eax存储着返回值
    asm volatile(
        "movl %esp, %eax\n"
        "andl $0xfffff000, %eax\n"
    );
}

void schedule() {
    task_t* cur = running_task();
    task_t* next = (cur == a ? b : a);
    task_switch(next);
}

u32 _ofp thread_a() {
    asm volatile("sti");
    while(true) {
        printk("A");
        delay(100000);
    }
}

u32 thread_b() {
    asm volatile("sti");
    while(true) {
        printk("B");
        delay(100000);
    }
}

static void task_create(task_t* task, target_t work) {
    u32 stack = (u32)task + STACK_SIZE;
    stack -= sizeof(task_frame_t);
    task_frame_t* frame = (task_frame_t*)stack;
    frame->ebx = 0x1111;
    frame->esi = 0x2222;
    frame->edi = 0x3333;
    frame->ebp = 0x4444;
    frame->eip = (void*)work;

    task->stack = (u32*)stack;
}

void task_init() {
    DEBUGK("init task!!!\n");
    task_create(a, thread_a);
    task_create(b, thread_b);
    schedule();
}