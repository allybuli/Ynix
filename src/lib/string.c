#include "../include/ynix/string.h"

char* strcpy(char* dest, const char* src) {
    char* ptr = dest;
    while(true) {
        *ptr ++ = *src;
        if(EOS == *src ++) {
            break;
        }
    }
    return dest;
}

char* strncpy(char* dest, const char* src, size_t count) {
    char* ptr = dest;
    for(size_t i = 0; i < count; i++) {
        *ptr ++ = *src;
        if(EOS == *src ++) {
            break;
        }
    }
    dest[count - 1] = EOS;
    return dest;
}

char* strcat(char* dest, const char* src) {
    char* ptr = dest;
    while (EOS != *ptr) {
        ptr ++;
    }
    while(true) {
        *ptr ++ = *src;
        if(EOS == *src ++) {
            break;
        }
    }
    return dest;
}

size_t strlen(const char *str) {
    char* ptr = (char*)str;
    while(EOS != *ptr) {
        ptr ++;
    }
    return ptr - str;
}

int strcmp(const char *lhs, const char *rhs) {
    while(*lhs == *rhs && EOS != *lhs && EOS != *rhs) {
        lhs ++;
        rhs ++;
    }
    if(EOS == *lhs && EOS == *rhs) {
        return 0;
    }
    return *lhs > *rhs ? 1 : -1;
}

char *strchr(const char *str, int ch) {
    char *ptr = (char*) str;
    while(true) {
        if(ch == *ptr) {
            return ptr;
        }
        if(EOS == *ptr ++) {
            return NULL;
        }
    }
}

char *strrchr(const char *str, int ch) {
    char* last = NULL;
    char* ptr = (char*)str;
    while(true) {
        if(ch == *ptr) {
            last = ptr;
        }
        if(EOS == *ptr ++) {
            return last;
        }
    }
}

int memcmp(const void *lhs, const void *rhs, size_t count) {
    char* lptr = (char*)lhs;
    char* rptr = (char*)rhs;
    while((0 < count) && *lptr == *rptr) {
        lptr ++;
        rptr ++;
        count --;
    }
    if(0 == count) {
        return 0;
    }
    return *lptr > *rptr ? 1 : -1;
}

void *memset(void *dest, int ch, size_t count) {
    char* ptr = (char*)dest;
    while(count --) {
        *ptr ++ = ch;
    }
    return dest;
}

void *memcpy(void *dest, const void *src, size_t count) {
    char* ptr = (char*)dest;
    while(count --) {
        *ptr ++ = *((char*)(src++));
    }
    return dest;
}

void *memchr(const void *str, int ch, size_t count) {
    char* ptr = (char*)str;
    while(count --) {
        if(ch == *ptr) {
            return (void*)ptr;
        }
        ptr ++;
    }
}

#define SEPARATOR1 '/'                                       // 目录分隔符 1
#define SEPARATOR2 '\\'                                      // 目录分隔符 2
#define IS_SEPARATOR(c) (c == SEPARATOR1 || c == SEPARATOR2) // 字符是否位目录分隔符

// 获取第一个分隔符
char *strsep(const char *str) {
    char* ptr = (char*)str;
    while(true) {
        if(IS_SEPARATOR(*ptr)) {
            return ptr;
        }
        if(EOS == *ptr ++) {
            return NULL;
        }
    }
}

// 获取最后一个分隔符
char *strrsep(const char *str) {
    char* last = NULL;
    char* ptr = (char*)str;
    while(true) {
        if(IS_SEPARATOR(*ptr)) {
            last = ptr;
        }
        if(EOS == *ptr) {
            return last;
        }
    }
}