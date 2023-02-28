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
#include "../include/ynix/global.h"
#include "../include/ynix/arena.h"

extern bitmap_t kernel_map;
extern tss_t tss;
extern void task_switch(task_t* next);

#define NR_TASKS 64
static task_t *task_table[NR_TASKS];
static list_t block_list; // 任务默认阻塞链表
static list_t sleep_list; // 任务睡眠链表
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

extern u32 volatile jiffies;
extern u32 jiffy;

void task_sleep(u32 ms) {
    assert(!get_interrupt_state());
    u32 ticks = ms / jiffy + (ms % jiffy != 0);
    ticks = ticks > 0 ? ticks : 1;

    task_t* cur = running_task();
    cur->ticks = jiffies + ticks;
    
    // 将任务插入睡眠链表
    list_t* list = &sleep_list;
    list_node_t* target_node = &list->tail;
    for(list_node_t* ptr = list->head.next; ptr != &list->tail; ptr = ptr->next) {
        task_t* task = element_entry(task_t, node, ptr);
        if(task->ticks > cur->ticks) {
            target_node = ptr;
            break;
        }
    }

    assert(cur->node.next == NULL);
    assert(cur->node.prev == NULL);

    list_insert_before(target_node, &cur->node);

    cur->state = TASK_SLEEPING;
    schedule();
}

void task_wakeup() {
    assert(!get_interrupt_state());

    list_t* list = &sleep_list;
    for(list_node_t* ptr = list->head.next; ptr != &list->tail;) {
        task_t* task = element_entry(task_t, node, ptr);
        if(task->ticks > jiffies) {
            break;
        }
        // unblock 会将指针清空
        ptr = ptr->next;
        
        task->ticks = 0;
        task_unblock(task);
    }
}

// 激活任务
void task_activate(task_t* task) {
    assert(task->magic == YNIX_MAGIC);

    // 每个任务都有自己的页目录
    if(task->pde != get_cr3()) {
        set_cr3(task->pde);
    }

    if(task->uid != KERNEL_USER) {
        tss.esp0 = (u32)task + PAGE_SIZE;
    }
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

    task_activate(next);
    task_switch(next);
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

// 调用该函数的地方不能有任何局部变量
// 调用前栈顶需要准备足够的空间
void task_to_user_mode(target_t target) {
    task_t* task = running_task();

    // 创建用户进程虚拟内存位图
    task->vmap = kmalloc(sizeof(bitmap_t));
    void* buf = alloc_kpage(1);
    bitmap_init(task->vmap, buf, PAGE_SIZE, KERNEL_MEMORY_SIZE/ PAGE_SIZE);
    // 创建用户进程页目录
    task->pde = copy_pde();
    set_cr3(task->pde);

    u32 addr = (u32)task + PAGE_SIZE;
    addr -= sizeof(intr_frame_t);
    intr_frame_t* iframe = (intr_frame_t*)(addr);

    iframe->vector = 0x20;
    iframe->edi = 1;
    iframe->esi = 2;
    iframe->ebp = 3;
    iframe->esp_dummy = 4;
    iframe->ebx = 5;
    iframe->edx = 6;
    iframe->ecx = 7;
    iframe->eax = 8;

    iframe->gs = 0;
    iframe->ds = USER_DATA_SELECTOR;
    iframe->es = USER_DATA_SELECTOR;
    iframe->fs = USER_DATA_SELECTOR;
    iframe->ss = USER_DATA_SELECTOR;
    iframe->cs = USER_CODE_SELECTOR;

    iframe->error = YNIX_MAGIC;
    iframe->eip = (u32)target;
    iframe->eflags = (0 << 12 | 0b10 | 1 << 9);
    iframe->esp = USER_STACK_TOP;

    asm volatile(
        "movl %0, %%esp\n"
        "jmp interrupt_exit\n"
        ::"m"(iframe)
    );
}

static void task_setup() {
    task_t* task = running_task();
    task->magic = YNIX_MAGIC;
    task->ticks = 1;

    memset(task_table, 0, sizeof(task_table));
}

extern void idle_thread();
extern void init_thread();
extern void test_thread();

void task_init() {
    list_init(&block_list);
    list_init(&sleep_list);
    task_setup();

    idle_task = task_create(idle_thread, "idle", 1, KERNEL_USER);
    task_create(init_thread, "init", 5, NORMAL_USER);
    task_create(test_thread, "test", 5, KERNEL_USER);
}