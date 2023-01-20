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