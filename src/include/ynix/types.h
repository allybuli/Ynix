#ifndef YNIX_TYPES_H
#define YNIX_TYPES_H

#define EOF -1
#define NULL ((void*)0)
#define EOS '\0'

#define bool _Bool
#define true 1
#define false 0

// 告诉编译器取消结构在编译过程中的优化对齐，按照实际占用字节数进行对齐,这样不会存在内存间隙
#define _packed __attribute__((packed))

// 用于省略函数的栈帧
#define _ofp __attribute__((optimize("omit-frame-pointer")))

typedef unsigned int size_t;

typedef char int8;
typedef short int16;
typedef int int32;
typedef long long int64;

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef unsigned long long u64;

typedef u32 idx_t;

#endif