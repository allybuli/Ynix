#include "../include/ynix/interrupt.h"
#include "../include/ynix/assert.h"
#include "../include/ynix/debug.h"
#include "../include/ynix/io.h"

#define LOGK(fmt, args...) DEBUGK(fmt, ##args)

#define KEYBOARD_DATA_PORT 0x60
#define KEYBOARD_CTRL_PORT 0x64

void keyboard_handler(int vector) {
    assert(vector == 0x21);
    send_eoi(vector);

    u16 scancode = inb(KEYBOARD_DATA_PORT);
    LOGK("keyboard input 0x%x\n", scancode);
}

void keyboard_init() {
    set_interrupt_handler(IRQ_KEYBOARD, keyboard_handler);
    set_interrupt_mask(IRQ_KEYBOARD, true);
}