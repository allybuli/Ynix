[bits 32]

extern kernel_init

global _start
_start:
    xchg bx, bx
    call kernel_init
    ; mov byte [0xb8000], 'K'
    xchg bx, bx
    jmp $