#ifndef YXIN_IO_H
#define YXIN_IO_H

#include "../ynix/types.h"

// 输入：读取外部设备中某个地址上的数据，将其写到CPU的某个寄存器上
// 输出：将CPU某个寄存器上的数据，写到外部设备中的某个地址上

extern u8 inb(u16 port);  // 输入一个字节
extern u16 inw(u16 port); // 输入一个字

extern void outb(u16 port, u8 value);  // 输出一个字节
extern void outw(u16 port, u16 value); // 输出一个字

#endif