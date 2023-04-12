#ifndef YNIX_SYSCALL_H
#define YNIX_SYSCALL_H

#include "types.h"
#include "fs.h"

typedef enum syscall_t {
    SYS_NR_TEST,
    SYS_NR_BRK,
    SYS_NR_READ,
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
    SYS_NR_LINK,
    SYS_NR_UNLINK,
    SYS_NR_OPEN,
    SYS_NR_CREAT,
    SYS_NR_CLOSE,
    SYS_NR_LSEEK,
    SYS_NR_CHDIR,
    SYS_NR_CHROOT,
    SYS_NR_GETCWD,
    SYS_NR_READDIR,
    SYS_NR_CLEAR,
} syscall_t;

u32 test();
void yield();
void sleep(u32 ms);
// 读文件
int read(fd_t fd, char *buf, int len);
// 写文件
int write(fd_t fd, char *buf, int len);
// 设置文件偏移量
int lseek(fd_t fd, off_t offset, int whence);
int32 brk(void*);
pid_t getpid();
pid_t getppid();
pid_t fork();
void exit();
pid_t waitpid(pid_t, int32*);
mode_t umask(mode_t mask);
int mkdir(char *pathname, int mode);
int rmdir(char *pathname);
int link(char *oldname, char *newname);
int unlink(char *filename);
fd_t open(char* pathname, int flags, int mode);
fd_t creat(char* pathname, int mode);
void close(fd_t fd);
// 获取当前路径
char* getcwd(char* buf, size_t size);
// 切换当前目录
int chdir(char* pathname);
// 切换根目录
int chroot(char* pathname);

int readdir(fd_t fd, dentry_t* dir, int count);
void clear();

#endif