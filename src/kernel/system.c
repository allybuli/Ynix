#include "../include/ynix/task.h"

mode_t sys_umask(mode_t mask) {
    task_t* task = running_task();
    mode_t old = task->umask;
    task->umask = mask;
    return old;
}