BITS 32 

SYS_EXIT equ 0x01        

global _start

section .text

_start:
times 0x5000 nop

    mov ebx, 0
    mov eax, SYS_EXIT
    syscall
