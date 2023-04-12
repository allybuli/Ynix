#include "../include/ynix/stdlib.h"

void delay(u32 count) {
    while(count --);
}

void hang() {
    while(true);
}

u32 div_round_up(u32 num, u32 size) {
    return (num + size - 1) / size;
}

// 只能处理纯数字串，或首位为负号的数字串
int atoi(const char* str) {
    if(!str) {
        return 0;
    }
    bool flag = true; // 正
    idx_t idx = 0;
    if(str[idx] == '-') {
        flag = false; // 负
        idx ++;
    }
    int ret = 0;
    while(str[idx]) {
        int num = str[idx] - '0';
        ret = ret * 10 + num;
        idx ++;
    }
    return flag ? ret : -ret;
}