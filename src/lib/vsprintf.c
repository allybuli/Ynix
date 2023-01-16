/* vsprintf.c -- Lars Wirzenius & Linus Torvalds. */
/*
 * Wirzenius wrote this portably, Torvalds fucked it up :-)
 */

#include "../include/ynix/stdio.h"
#include "../include/ynix/string.h"
#include "../include/ynix/assert.h"

#define ZEROPAD 1  // 填充零
#define SIGN 2     // unsigned/signed long
#define PLUS 4     // 显示加
#define SPACE 8    // 如是加，则置空格
#define LEFT 16    // 左调整
#define SPECIAL 32 // 0x
#define SMALL 64   // 使用小写字母

#define is_digit(c) ((c) >= '0' && (c) <= '9')

static int skip_atoi(const char** s) {
    int i = 0;
    while(is_digit(**s)) {
        i = i * 10 + (**s) - '0';
        (*s) ++;
    }
    return i;
}

static char* number(char* str, unsigned long num, int base, int size, int precision, int flags) {
    char c, sign, tmp[36];
    const char* digits = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    int i, index;
    char* ptr = str;

    if(flags & SMALL) {
        digits = "0123456789abcdefghijklmnopqrstuvwxyz";
    }
    if(flags & LEFT) {
        flags &= ~ZEROPAD;
    }
    if(base < 2 | base > 36) {
        return 0;
    }
    
    c = (flags & ZEROPAD) ? '0' : ' ';
    if(flags & SIGN && num < 0) {
        sign = '-';
        num = -num;
    } else {
        sign = (flags & PLUS) ? '+' : ((flags & SPACE) ? ' ' : 0);
    }
    
    if(sign) {
        size --;
    }
    if(flags & SPACE) {
        if(16 == base) {
            size -= 2;
        } else if(8 == base) {
            size --;
        }
    }

    i = 0;
    if(0 == num) {
        tmp[i++] = '0';
    } else {
        while(0 != num) {
            index = num % base;
            num /= base;
            tmp[i++] = digits[index];
        }
    }
    if(i > precision) {
        precision = i;
    }
    size -= precision;

    if(!(flags & (ZEROPAD + LEFT))) {
        while(size -- > 0) {
            *str ++ = ' ';
        }
    }
    if(sign) {
        *str ++ = sign;
    }
    if(flags & SPECIAL) {
        if(8 == base) {
            *str ++ = '0';
        } else if(16 == base) {
            *str ++ = '0';
            *str ++ = digits[33];
        }
    }
    if(!(flags & LEFT)) {
        while(size -- > 0) {
            *str ++ = c;
        }
    }
    while(i < precision --) {
        *str ++ = '0';
    }
    while(i -- > 0) {
        *str ++ = tmp[i];
    }
    while(size -- > 0) {
        *str ++ = ' ';
    }
    return str;
}

int vsprintf(char* buf, const char* fmt, va_list args) {
    int len;
    int i;

    // 用于存放转换过程中的字符串
    char* str;
    char* s;
    int* ip;

    // number() 函数使用标志
    int flags;
    int field_width;
    int precision;
    int qualifier;
    for(str = buf; *fmt; fmt ++) {
        if('%' != *fmt) {
            *str ++ = *fmt;
            continue;
        }

        flags = 0;
        while(*fmt) {
            fmt ++;
            bool f = true;
            switch(*fmt) {
                case '-':
                    flags |= LEFT;
                    break;
                case '+':
                    flags |= PLUS;
                    break;
                case ' ':
                    flags |= SPACE;
                    break;
                case '#':
                    flags |= SPECIAL;
                    break;
                case '0':
                    flags |= ZEROPAD;
                    break;
                default:
                    f = false;
            }
            if(!f) {
                break;
            }
        }

        field_width = -1;
        if(is_digit(*fmt)) {
            field_width = skip_atoi(&fmt);
        } else if('*' == *fmt) {
            fmt ++;
            field_width = va_arg(args, int);
            if(0 > field_width) {
                field_width = -field_width;
                flags |= LEFT;
            }
        }

        precision = -1;
        if('.' == *fmt) {
            fmt ++;
            if(is_digit(*fmt)) {
                precision = skip_atoi(&fmt);
            } else if('*' == *fmt) {
                precision = va_arg(args, int);
            }
            if(0 > precision) {
                precision = 0;
            }
        }

        qualifier = -1;
        if('h' == *fmt || 'l' == *fmt || 'L' == *fmt) {
            qualifier = *fmt;
            fmt ++;
        }

        switch (*fmt) {
            case 'c':
                if(!(flags & LEFT)) {
                    while(-- field_width > 0) {
                        *str ++ = ' ';
                    }
                }
                *str ++ = (unsigned char)va_arg(args, int);
                while(-- field_width > 0) {
                    *str ++ = ' ';
                }
                break;
            case 's':
                s = va_arg(args, char*);
                len = strlen(s);
                if(precision < 0) {
                    precision = len;
                } else if(precision < len) {
                    len = precision;
                }
                if(!(flags & LEFT)) {
                    while(len < field_width --) {
                        *str ++ = ' ';
                    }
                }
                for(i = 0; i < len; i++) {
                    *str ++ = *s ++;
                }
                while(len < field_width --) {
                    *str ++ = ' ';
                }
                break;
            case 'o':
                str = number(str, va_arg(args, unsigned long), 8, field_width, precision, flags);
                break;
            case 'p':
                if(-1 == field_width) {
                    field_width = 8;
                    flags |= ZEROPAD;
                }
                str = number(str, (unsigned long)va_arg(args, void*), 16, field_width, precision, flags);
                break;
            case 'x':
                flags |= SMALL;
            case 'X':
                str = number(str, va_arg(args, unsigned long), 16, field_width, precision, flags);
                break;
            case 'd':
            case 'i':
                flags |= SIGN;
            case 'u':
                str = number(str, va_arg(args, unsigned long), 10, field_width, precision, flags);
                break;
            case 'n':
                ip = va_arg(args, int*);
                *ip = (str - buf);
                break;
            default:
                if('%' != *fmt) {
                    *str ++ = '%';
                }
                if(*fmt) {
                    *str ++ = *fmt;
                } else {
                    fmt --;
                }
                break;
        }
    }
    *str = '\0';
    i = str - buf;
    assert(i < 1024);
    return i;
}

// 结果按格式输出字符串到 buf
int sprintf(char* buf, const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    int i = vsprintf(buf, fmt, args);
    va_end(args);
    return i;
}