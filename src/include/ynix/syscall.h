#ifndef YNIX_SYSCALL_H
#define YNIX_SYSCALL_H

#include "types.h"

typedef enum syscall_t {
    SYS_NR_TEST,
    SYS_NR_BRK,
    SYS_NR_WRITE,
    SYS_NR_SLEEP,
    SYS_NR_YIELD,
    SYS_NR_GETPID,
    SYS_NR_GETPPID,
    SYS_NR_FORK,
    SYS_NR_EXIT,
    SYS_NR_WAITPID,
    SYS_NR_UMASK,
    SYS_NR_MKDIR,
    SYS_NR_RMDIR,
} syscall_t;

u32 test();
void yield();
void sleep(u32 ms);
int32 write(fd_t, char*, u32);
int32 brk(void*);
pid_t getpid();
pid_t getppid();
pid_t fork();
void exit();
pid_t waitpid(pid_t, int32*);
mode_t umask(mode_t mask);
int mkdir(char *pathname, int mode);
int rmdir(char *pathname);

#endif