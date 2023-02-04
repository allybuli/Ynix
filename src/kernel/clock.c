#include "../include/ynix/clock.h"
#include "../include/ynix/interrupt.h"
#include "../include/ynix/assert.h"
#include "../include/ynix/task.h"
#include "../include/ynix/ynix.h"
#include "../include/ynix/debug.h"

#define PIT_CHAN0_REG 0X40
#define PIT_CHAN2_REG 0X42
#define PIT_CTRL_REG 0X43

#define HZ 100
#define OSCILLATOR 1193182
#define CLOCK_COUNTER (OSCILLATOR / HZ)
#define JIFFY (1000 / HZ)

// 时间片计数器
u32 volatile jiffies = 0;

void clock_handler(int vector) {
    assert(vector == 0x20);
    send_eoi(vector);

    jiffies++;
    if(jiffies % 1000 == 0) DEBUGK("clock jiffies %d ...\n", jiffies);
}

// 计数器初始化，通过计数器0可以控制时钟信号的频率，进而控制一个时间片的长短
void pit_init() {
    outb(PIT_CTRL_REG, 0b00110100);
    outb(PIT_CHAN0_REG, CLOCK_COUNTER & 0xff);
    outb(PIT_CHAN0_REG, (CLOCK_COUNTER >> 8) & 0xff);
}

void clock_init() {
    pit_init();
    set_interrupt_handler(IRQ_CLOCK, clock_handler);
    set_interrupt_mask(IRQ_CLOCK, true);
    DEBUGK("clock_init!!!\n");
}