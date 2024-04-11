SECTION .data
msg db "HelloWorld!!!" , 0ah , 0dh  ;0ah(ASCII码：换行符)，odh(ASCII码：回车符)
msglen equ ($ - msg)

SECTION .text
global _start

_start:
    mov edx,1
    mov ecx,msg
    mov ebx,1d
    mov eax,4d
    int 80h  ;调用linux系统中断，在屏幕上输出字符串

    mov ebx,0d  ;表示正常退出
    mov eax,1d
    int 80h  ;调用 SYS_EXIT ，正常退出 