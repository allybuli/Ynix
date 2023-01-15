#include "../include/ynix/interrupt.h"
#include "../include/ynix/global.h"
#include "../include/ynix/debug.h"

gate_t idt[IDT_SIZE];
pointer_t idt_ptr;

extern void interrupt_handler();

void interrupt_init() {
    for(size_t i = 0; i < IDT_SIZE; i++) {
        idt[i].offset0 = (u32)interrupt_handler & 0xffff;
        idt[i].offset1 = ((u32)interrupt_handler >> 16) & 0xffff;
        idt[i].selector = 1 << 3;
        idt[i].reserved = 0;
        idt[i].type = 0b1110;
        idt[i].segment = 0;
        idt[i].DPL = 0;
        idt[i].present = 1;
    }
    idt_ptr.base = (u32)idt;
    idt_ptr.limit = sizeof(idt) - 1;
    asm volatile("lidt idt_ptr\n");
}