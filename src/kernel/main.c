#include "../include/ynix/ynix.h"

int magic = YNIX_MAGIC;
char buffer[1024];


void kernel_init() {
    char* video = (char*) 0xb8000;
    char message[] = "Hello Ynix!!!!";
    for(int i = 0; i < sizeof(message); i++) {
        video[2 * i] = message[i];
    }
}