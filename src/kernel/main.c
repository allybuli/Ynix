#include "../include/ynix/ynix.h"
#include "../include/ynix/io.h"
#include "../include/ynix/types.h"
#include "../include/ynix/string.h"
#include "../include/ynix/console.h"
#include "../include/ynix/stdarg.h"
#include "../include/ynix/printk.h"
#include "../include/ynix/assert.h"
#include "../include/ynix/debug.h"
#include "../include/ynix/global.h"
#include "../include/ynix/interrupt.h"
#include "../include/ynix/stdlib.h"
#include "../include/ynix/task.h"
#include "../include/ynix/memory.h"
#include "../include/ynix/bitmap.h"
#include "../include/ynix/debug.h"
#include "../include/ynix/clock.h"
#include "../include/ynix/syscall.h"

#define LOGK(fmt, args...) DEBUGK(fmt, ##args)

extern void keyboard_init();
extern void syscall_init();

void kernel_init() {
    interrupt_init();
    clock_init();
    keyboard_init();
    task_init();
    syscall_init();

    set_interrupt_state(true);
    return;
}
