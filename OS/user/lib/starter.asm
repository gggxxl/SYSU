[BITS 16]
extern main
global _start

[section .text]
_start:
	mov ax,cs
	mov ds,ax
	mov es,ax
	call dword main
	mov ah,00h
	int 21h

[section .data]
