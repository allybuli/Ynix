#ifndef YNIX_IDE_H
#define YNIX_IDE_H

#include "types.h"
#include "mutex.h"

#define SECTOR_SIZE 512  // 扇区大小

#define IDE_CTRL_NR 2    // 控制器数量
#define IDE_DISK_NR 2    // 每个控制器可挂磁盘数量

typedef struct ide_disk_t {
    char name[8];
    struct ide_ctrl_t* ctrl; // 控制器指针
    u8 selector;             // 磁盘选择
    bool master;             // 是否为主盘
    u32 total_lba;           // 可用扇区数量
    u32 cylinders;           // 柱面数
    u32 heads;               // 磁头数
    u32 sectors;             // 扇区数
} ide_disk_t;

typedef struct ide_ctrl_t {
    char name[8];
    lock_t lock;
    u16 iobase;
    ide_disk_t disks[IDE_DISK_NR];
    ide_disk_t* active;    // 当前选择的磁盘
    u8 control;            // 控制字节
    struct task_t* waiter; // 等待控制器唤醒的进程
} ide_ctrl_t;

int ide_pio_read(ide_disk_t* disk, void* buf, u8 count, idx_t lba);
int ide_pio_write(ide_disk_t* disk, void* buf, u8 count, idx_t lba);

#endif