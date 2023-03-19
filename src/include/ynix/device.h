#ifndef YNIX_DEVICE_H
#define YNIX_DEVICE_H

#include "types.h"

#define NAMELEN 16

// 设备类型
enum device_type_t {
    DEV_NULL,  // 空设备
    DEV_CHAR,  // 字符设备
    DEV_BLOCK, // 块设备
};

enum device_subtype_t {
    DEV_CONSOLE = 1,  // 控制台
    DEV_KEYBOARD,     // 键盘
};

typedef struct device_t {
    char name[NAMELEN];
    int type;
    int subtype;
    dev_t dev;
    dev_t parent;
    void *ptr;          // 设备指针
    // 设备控制
    int (*ioctl)(void *dev, int cmd, void *args, int flags);
    // 读设备
    int (*read)(void *dev, void *buf, size_t count, idx_t idx, int flags);
    // 写设备
    int (*write)(void *dev, void *buf, size_t count, idx_t idx, int flags);
} device_t;

// 安装设备
dev_t device_install(
    int type, int subtype,
    void *ptr, char *name, dev_t parent,
    void *ioctl, void *read, void *write);

// 根据子类型查找设备
device_t *device_find(int type, idx_t idx);

// 根据设备号查找设备
device_t *device_get(dev_t dev);

// 控制设备
int device_ioctl(dev_t dev, int cmd, void *args, int flags);

// 读设备
int device_read(dev_t dev, void *buf, size_t count, idx_t idx, int flags);

// 写设备
int device_write(dev_t dev, void *buf, size_t count, idx_t idx, int flags);

#endif