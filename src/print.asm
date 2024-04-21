; print.asm
SECTION .text
global  print
print:        ;输出字串
	push edx
	push ecx
	push ebx
	push eax
	push eax
	call strlen
	mov  edx, eax
	pop  eax
	mov  ecx, eax
	mov  ebx, 1
	mov  eax, 4
	int  80h
	pop  eax
	pop  ebx
	pop  ecx
	pop  edx
	ret
	
strlen: ;求字串长
	push ebx
	mov  ebx, eax
	strlennextchar:
		cmp byte [eax], 0
		jz  strlenfinished
		inc eax
		jmp strlennextchar
	strlenfinished:
		sub eax, ebx
		pop ebx
		ret

