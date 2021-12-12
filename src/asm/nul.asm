BITS 64

SYS_EXIT equ 0x3c        

global _start

section .text

_start:
times 0x5000 nop

    xor rdi, rdi
    mov rax, SYS_EXIT
    syscall
