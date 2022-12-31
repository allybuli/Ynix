#ifndef YNIX_STDARG_H
#define YXIN_STDARG_H

// 保存可变参数指针
typedef char* va_list;

// 启用可变参数
// ap是一个指针，指向倒数第二个压入栈内的参数，v是倒数第一个压入栈内的参数
#define va_start(ap, v) (ap = (va_list)&v + sizeof(char*))
// 获取下一个参数
// ap指针向后偏移一个单位长度，并返回偏移前所指向数据的值
#define va_arg(ap, type) (*(type*)((ap += sizeof(char*)) - sizeof(char*)))
// 结束可变参数
// 参数变量访问完成，ap指向空
#define va_end(ap) (ap = (va_list)0)

#endif