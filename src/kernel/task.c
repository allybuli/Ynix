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
#include "../include/ynix/fs.h"

#define LOGK(fmt, args...) DEBUGK(fmt, ##args)

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
            memset(task_table[i], 0, PAGE_SIZE);
            task_table[i]->pid = i;
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
    list_insert_sort(&sleep_list, &cur->node, element_node_offset(task_t, node, ticks));

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
    task->brk = KERNEL_MEMORY_SIZE; // todo
    task->iroot = get_root_inode();
    task->ipwd = get_root_inode();
    task->magic = YNIX_MAGIC;

    return task;
}

// 调用该函数的地方不能有任何局部变量
// 调用前栈顶需要准备足够的空间
void task_to_user_mode(target_t target) {
    task_t* task = running_task();

    // 创建用户进程虚拟内存位图
    task->vmap = kmalloc(sizeof(bitmap_t));
    void* buf = (void*)alloc_kpage(1);
    bitmap_init(task->vmap, buf, PAGE_SIZE, KERNEL_MEMORY_SIZE/ PAGE_SIZE);
    // 创建用户进程页目录
    task->pde = (u32)copy_pde();
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

pid_t sys_getpid() {
    task_t* task = running_task();
    return task->pid;
}

pid_t sys_getppid() {
    task_t* task = running_task();
    return task->ppid;
}

extern void interrupt_exit();

static void task_build_stack(task_t* task) {
    u32 addr = (u32)task + PAGE_SIZE;
    addr -= sizeof(intr_frame_t);
    intr_frame_t* iframe = (intr_frame_t*)addr;
    // fork调用子进程返回0
    iframe->eax = 0;

    addr -= sizeof(task_frame_t);
    task_frame_t* tframe = (task_frame_t*)addr;
    tframe->ebp = 0xaa55aa55;
    tframe->ebx = 0xaa55aa55;
    tframe->edi = 0xaa55aa55;
    tframe->esi = 0xaa55aa55;
    tframe->eip = interrupt_exit;
    task->stack = (u32*)tframe;
}

pid_t task_fork() {
    // 1、复制进程资源
    // 2、子进程设为就绪状态(子进程开始执行)
    task_t* task = running_task();
    assert(task->state == TASK_RUNNING && task->node.next == NULL && task->node.prev == NULL);

    // 拷贝内核栈和PCB
    task_t* child = get_free_task();
    pid_t child_pid = child->pid;
    memcpy(child, task, PAGE_SIZE);
    child->pid = child_pid;
    child->ppid = task->pid;
    child->ticks = child->priority;
    child->state = TASK_READY;

    // 拷贝虚拟内存位图(深拷贝)
    child->vmap = kmalloc(sizeof(bitmap_t));
    memcpy(child->vmap, task->vmap, sizeof(bitmap_t));

    // 拷贝虚拟位图缓存
    void* buf = (void*)alloc_kpage(1);
    // todo free_kpage
    memcpy(buf, task->vmap->bits, PAGE_SIZE);
    child->vmap->bits = buf;

    // 拷贝页目录
    child->pde = (u32)copy_pde();

    // 构造 child 内核栈
    task_build_stack(child); // ROP

    // fork调用父进程返回子进程的id
    return child->pid;
}

void task_exit(int status) {
    // 不返回，进程结束后直接调度其它进程执行
    // 1、释放资源
    // 2、将当前进程的子进程托管给当前进程的父进程
    task_t* task = running_task();
    assert(task->state == TASK_RUNNING && task->node.next == NULL && task->node.prev == NULL);
    task->state = TASK_DIED;
    task->status = status;

    free_pde();
    free_kpage((u32)task->vmap->bits, 1);
    kfree(task->vmap);

    // 0号进程是基本进程(空转)
    // 1号进程是init进程
    for(size_t i = 2; i < NR_TASKS; i++) {
        if(NULL == task_table[i]) {
            continue;
        }
        if(task_table[i]->ppid == task->pid) {
            task_table[i]->ppid = task->ppid;
        }
    }
    LOGK("Task 0x%p exit\n", task);
    // pid_t pid = task->pid;
    // free_kpage((u32)task, 1);
    // task_table[pid] = NULL;

    task_t* parent = task_table[task->ppid];
    if(TASK_WAITING == parent->state && parent->waitpid == task->pid) {
        task_unblock(parent);
    }
    schedule();
}

pid_t task_waitpid(pid_t pid, int32* status) {
    // 寻找进程号为pid的子进程
    // 不存在，返回-1
    // 目标子进程未死亡
    if(pid < 0 || pid > NR_TASKS) return -1;
    if(NULL == task_table[pid]) return -1;
    task_t* task = running_task();
    if(task_table[pid]->ppid != task->pid) return -1;
    while(task_table[pid]->state != TASK_DIED) {
        task->waitpid = pid;
        task_block(task, NULL, TASK_WAITING);
        if(NULL == task_table[pid]) break;
    }
    assert(NULL != task_table[pid]);
    *status = task_table[pid]->status;
    free_kpage((u32)task_table[pid], 1);
    task_table[pid] = NULL;
    return pid;
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