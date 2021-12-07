BITS 64


;--------- CONSTANTS ---------; 

; file syscall numbers
SYS_WRITE   equ 0x1         
SYS_READ    equ 0x0         
SYS_CLOSE   equ 0x3         
SYS_EXIT    equ 0x3c        
SYS_OPEN    equ 0x2         

; open macros
O_RDONLY equ 0x0
O_WRONLY equ 0x1
O_CREAT  equ 0x40
BUFSIZ   equ 0x2000
;--------- CONSTANTS ---------; 

global _start

; constants
section .data
times 0x5000 resd 0 

; variables 
section .bss
times 0x5000 resd 0 

; code
section .text

; arg1 = rdi
; arg2 = rsi
; arg3 = rdx
; arg4 = rcx
; arg5 = r8

_start:

times 0x5000 nop

    mov rdi, 0
    mov rax, SYS_EXIT
    syscall
