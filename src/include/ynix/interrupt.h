#ifndef YNIX_INTERRUPT_H
#define YNIX_INTERRUPT_H

#include "types.h"

#define IDT_SIZE 256

#define IRQ_CLOCK 0      // 时钟
#define IRQ_KEYBOARD 1   // 键盘

#define IRQ_MASTER_NR 0x20 // 主片起始向量号
#define IRQ_SLAVE_NR 0x28  // 从片起始向量号

typedef struct gate_t {
    u16 offset0;
    u16 selector;
    u8 reserved;
    u8 type : 4;
    u8 segment : 1;
    u8 DPL : 2;
    u8 present : 1;
    u16 offset1;
} _packed gate_t;

typedef void* handler_t; // 中断处理函数

void interrupt_init();

void send_eoi(int vector);

// 设置中断处理函数
void set_interrupt_handler(u32 irq, handler_t handler);
void set_interrupt_mask(u32 irq, bool enable);

bool interrupt_disable();
bool get_interrupt_state();
void set_interrupt_state(bool state);

#endif