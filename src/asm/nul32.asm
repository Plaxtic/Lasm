BITS 32 

SYS_EXIT equ 0x01        

global _start

section .text

_start:
times 0x5000 nop

    xor ebx, ebx
    mov eax, SYS_EXIT
    int 0x80 
