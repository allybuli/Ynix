#include "../include/ynix/task.h"
#include "../include/ynix/printk.h"
#include "../include/ynix/debug.h"
#include "../include/ynix/stdlib.h"
#include "../include/ynix/ynix.h"
#include "../include/ynix/interrupt.h"
#include "../include/ynix/memory.h"
#include "../include/ynix/bitmap.h"
#include "../include/ynix/string.h"
#include "../include/ynix/assert.h"
#include "../include/ynix/syscall.h"
#include "../include/ynix/list.h"

extern bitmap_t kernel_map;
extern void task_switch(task_t* next);

#define NR_TASKS 64
static task_t *task_table[NR_TASKS];
static list_t block_list; // 任务默认阻塞链表
static task_t* idle_task; // 基础任务

static task_t *get_free_task() {
    for (size_t i = 0; i < NR_TASKS; i++) {
        if (task_table[i] == NULL) {
            task_table[i] = (task_t *)alloc_kpage(1); // todo free_kpage
            return task_table[i];
        }
    }
    panic("No more tasks");
}

static task_t* task_search(task_state_t state) {
    assert(!get_interrupt_state());
    task_t* next = NULL;
    task_t* cur = running_task();
    for(size_t i = 0; i < NR_TASKS; i++) {
        task_t* task = task_table[i];
        if(NULL == task || state != task->state || cur == task) {
            continue;
        }
        if(NULL == next || task->ticks > next->ticks || task->jiffies < next->jiffies) {
            next = task;
        }
    }
    if(NULL == next && TASK_READY == state) {
        // 系统无可调度的任务，启动基础任务
        next = idle_task;
    }
    return next;
}

task_t* running_task() {
    // 返回栈帧的栈顶指针，eax存储着返回值
    asm volatile(
        "movl %esp, %eax\n"
        "andl $0xfffff000, %eax\n"
    );
}

void task_yield() {
    schedule();
}

void task_block(task_t* task, list_t* blist, task_state_t state) {
    assert(!get_interrupt_state());
    assert(NULL == task->node.prev);
    assert(NULL == task->node.next);
    if(NULL == blist) {
        blist = &block_list;
    }
    list_push(blist, &task->node);

    assert(state != TASK_READY && state != TASK_RUNNING);
    task->state = state;
    task_t* cur = running_task();
    if(cur == task) {
        schedule();
    }
}

void task_unblock(task_t* task) {
    assert(!get_interrupt_state());
    list_remove(&task->node);

    assert(NULL == task->node.prev);
    assert(NULL == task->node.next);
    task->state = TASK_READY;
}

void schedule() {
    assert(!get_interrupt_state());
    task_t *cur = running_task();
    task_t *next = task_search(TASK_READY);

    assert(next != NULL);
    assert(next->magic == YNIX_MAGIC);

    if(cur->state == TASK_RUNNING) {
        cur->state = TASK_READY;
    }
    if(!cur->ticks) {
        cur->ticks = cur->priority;
    }
    next->state = TASK_RUNNING;
    if(next == cur) {
        return;
    }

    task_switch(next);
}

u32 thread_a() {
    set_interrupt_state(true);
    while(true) {
        printk("A");
        test();
    }
}

u32 thread_b() {
    set_interrupt_state(true);
    while(true) {
        printk("B");
        test();
    }
}

u32 thread_c() {
    set_interrupt_state(true);
    while(true) {
        printk("C");
        test();
    }
}

static task_t *task_create(target_t target, const char *name, u32 priority, u32 uid)
{
    task_t *task = get_free_task();
    memset(task, 0, PAGE_SIZE);

    u32 stack = (u32)task + PAGE_SIZE;

    stack -= sizeof(task_frame_t);
    task_frame_t *frame = (task_frame_t *)stack;
    frame->ebx = 0x11111111;
    frame->esi = 0x22222222;
    frame->edi = 0x33333333;
    frame->ebp = 0x44444444;
    frame->eip = (void *)target;

    strcpy((char*)task->name, name);

    task->stack = (u32 *)stack;
    task->priority = priority;
    task->ticks = task->priority;
    task->jiffies = 0;
    task->state = TASK_READY;
    task->uid = uid;
    task->vmap = &kernel_map;
    task->pde = KERNEL_PAGE_DIR;
    task->magic = YNIX_MAGIC;

    return task;
}

static void task_setup() {
    task_t* task = running_task();
    task->magic = YNIX_MAGIC;
    task->ticks = 1;

    memset(task_table, 0, sizeof(task_table));
}

extern u32 idle_thread();
extern u32 init_thread();

void task_init() {
    list_init(&block_list);
    task_setup();

    task_create(idle_thread, "idle", 1, KERNEL_USER);
    task_create(init_thread, "init", 5, NORMAL_USER);
}