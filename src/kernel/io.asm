[bits 32]
section .text
global inb
inb:
    push ebp
    mov ebp, esp

    xor eax, eax
    mov edx, [ebp + 8]
    in al, dx
    
    nop
    nop
    nop

    leave
    ret

global inw
inw:
    push ebp
    mov ebp, esp

    xor eax, eax
    mov edx, [ebp + 8]
    in ax, dx
    
    nop
    nop
    nop

    leave
    ret

global outb
outb:
    push ebp
    mov ebp, esp

    mov edx, [ebp + 8]
    mov eax, [ebp + 12]
    out dx, al

    nop
    nop
    nop

    leave
    ret

global outw
outw:
    push ebp
    mov ebp, esp

    mov edx, [ebp + 8]
    mov eax, [ebp + 12]
    out dx, ax

    nop
    nop
    nop

    leave
    ret