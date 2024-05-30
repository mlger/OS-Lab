
; ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
;                               syscall.asm
; ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
;                                                     Forrest Yu, 2005
; ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

%include "sconst.inc"

_NR_get_ticks       equ 0 ; 要跟 global.c 中 sys_call_table 的定义相对应！
_NR_sleep_ms		equ 1
_NR_print_str		equ 2
_NR_P				equ 3
_NR_V				equ 4
_NR_work			equ 5
INT_VECTOR_SYS_CALL equ 0x90

; 导出符号
global	get_ticks
global	sleep_ms
global	print_str
global	P
global	V
global	work


bits 32
[section .text]

; ====================================================================
;                              get_ticks
; ====================================================================
get_ticks:
	mov	eax, _NR_get_ticks
	int	INT_VECTOR_SYS_CALL
	ret

; ====================================================================
;                              sleep_ms(int milli_sec);
; ====================================================================
sleep_ms:
	mov eax, _NR_sleep_ms
	push ebx
	mov ebx, [esp + 8]	; milli_sec
	int INT_VECTOR_SYS_CALL 
	pop ebx
	ret

; ====================================================================
;                              print_str(char* buf, col);
; ====================================================================
print_str:
	mov eax, _NR_print_str
	push ecx
	push ebx
	mov ebx, [esp + 12]	; buf
	mov ecx, [esp + 16] ; col
	int INT_VECTOR_SYS_CALL
	pop ebx
	pop ecx
	ret

; ====================================================================
;                              P(SEMAPHORE* sem);
; ====================================================================
P:
	mov eax, _NR_P
	push ebx
	mov ebx, [esp + 8]	; sem
	int INT_VECTOR_SYS_CALL
	pop ebx
	ret

; ====================================================================
;                              V(SEMAPHORE* sem);
; ====================================================================
V:
	mov eax, _NR_V
	push ebx
	mov ebx, [esp + 8]	; sem
	int INT_VECTOR_SYS_CALL
	pop ebx
	ret

; ====================================================================
;                              work(int x);
; ====================================================================
work:
	mov eax, _NR_work
	push ebx
	mov ebx, [esp + 8]	; x
	int INT_VECTOR_SYS_CALL
	pop ebx
	ret

; ====================================================================================
;                                 sys_call
; ====================================================================================
;sys_call:
;        call    save

;        sti

;		push ecx
;		push ebx
;        call    [sys_call_table + eax * 4]
;		pop	ebx
;		pop ecx

;        mov     [esi + EAXREG - P_STACKBASE], eax

;        cli

;        ret