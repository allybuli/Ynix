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
extern void tss_init();
extern void arena_init();
extern void ide_init();
extern void buffer_init();
extern void super_init();
extern void inode_init();

void kernel_init() {
    tss_init();
    arena_init();
    interrupt_init();
    clock_init();
    keyboard_init();
    ide_init();
    buffer_init();
    task_init();
    syscall_init();
    inode_init();
    super_init();

    set_interrupt_state(true);
    return;
}
