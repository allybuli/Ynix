#ifndef YNIX_DEBUG_H
#define YNIX_DEBUG_H

void debugk(char* file, int line, const char* fmt, ...);

// bochs 魔数断点
#define BMB asm volatile("xchg %bx, %bx");

#define DEBUGK(fmt, args...) debugk(__BASE_FILE__, __LINE__, fmt, ##args)

#endif