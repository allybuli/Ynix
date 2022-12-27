[bits 32]

extern exit

global main
main:
    push 5
    push eax

    pop ebx
    pop edx

    push 0
    call exit