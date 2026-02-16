BITS 64

global _start

section .text

_start:	mov rax, [rsp+8]
	mov bl, [rax]
	test bl, bl
	mov bl, [rax]
	inc rax
	test bl, bl
	jnz -7
	mov rbx, [rsp+8]
	sub rax, rbx
	mov rsi, rbx
	mov rdi, 1
	mov rdx, rax
	mov rax, 1
	syscall
