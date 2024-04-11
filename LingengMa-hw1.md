#### 1、实验任务一：Hello OS

##### 1.1 代码

```
;boot.asm
	org 07c00h
	mov ax, cs
	mov ds, ax
	mov es, ax
	call DispStr
	jmp $
DispStr:
	mov ax, BootMessage
	mov bp, ax
	mov cx, 10
	mov ax, 01301h
	mov bx, 000ch
	mov dl, 0
	int 10h
	ret

BootMessage: db "Hello, OS!"
times 510-($-$$) db 0
dw 0xaa55
```

`boshsrc`

```
megs:32
display_library: sdl2
floppya: 1_44=a.img, status=inserted
boot: floppy
romimage: file=$BXSHARE/BIOS-bochs-legacy
```



##### 1.2 运行截图

![image-20240401154927518](https://raw.githubusercontent.com/mlger/Pict/main/newPath/image-20240401154927518.png)

#### 2、实验任务二：进制转换

##### 2.1 代码

```
;hello.asm
;; nasm -f elf32 hello.asm -o hello.o && ld -m elf_i386 hello.o -o hello && ./hello
SECTION .bss
input:  resb 256


SECTION .data
	msginput: db "Please input:", 0h ;输入提示
	msgerror: db "error", 0h         ;error
	msg0x:    db "0x",0h
	msg0b:    db "0b",0h
	msg0o:    db "0o",0h
	num: resb 256 ;存储原数字
	res: resb 256 ;存储答案
	calcbase:  db 0h         ;存储基数
	myflag:    db 0h         ;状态机
	ismaximum: db 0h         ;判断是否等于1e30
	;input:    db "11 o", 0h



SECTION .text
global  _start

_start:
	call read        ;输入，整行存储于input中
	call solve_input

	;push eax
	;mov  eax,          myflag
	;add  byte[myflag], 48
	;call charprint
	;sub  byte[myflag], 48
	;pop  eax

	cmp byte[myflag], 0
	jz  _start
	cmp byte[myflag], 1
	jz  printerror
	cmp byte[myflag], 2
	jz  printerror
	cmp byte[myflag], 3
	jz  check_num
	cmp byte[myflag], 4
	jz  quit

	cmp byte[myflag], 5
	jz  printerror

solve_input: ;处理输入，提取原数num，目标进制base

	mov eax,          input
	mov byte[myflag], 0

	nextchar:
		cmp byte [eax], 0
		jz  finished
		cmp byte [eax], 10
		jz  finished
		cmp byte[eax],  13
		jz  finished

		;小小的调试
		;push eax
		;call charprint
		;pop eax

		;push eax
		;add byte[myflag], 48
		;mov eax, myflag
		;call charprint
		;sub byte[myflag], 48
		;pop  eax


		mov bl, byte[eax] ;ebx存储当前字符
		inc eax

		cmp byte[myflag], 4
		je  solve4
		cmp byte[myflag], 3
		je  solve3
		cmp byte[myflag], 2
		je  solve2
		cmp byte[myflag], 1
		je  solve1
		cmp byte[myflag], 0
		je  solve0


	solve0:
		cmp bl, 32
		je  nextchar
		cmp bl, 113   ;等于'q'
		je  set4
		cmp bl, 48
		jl  finisherr
		cmp bl, 57
		jg  finisherr
		jmp set1

	set1:
		mov byte[myflag], 1

 		mov ecx, num
		mov byte[ecx], bl
		inc ecx
		jmp nextchar

	solve1:
		cmp bl,        32
		je  set2
		cmp bl,        48
		jl  finisherr
		cmp bl,        57
		jg  finisherr
		mov byte[ecx], bl
		inc ecx
		jmp nextchar

	set2:
		mov byte[myflag], 2
		mov byte[ecx],    0
		jmp nextchar

	solve2:
		cmp bl, 32
		je  nextchar
		cmp bl, 98
		je  set3b
		cmp bl, 111
		je  set3o
		cmp bl, 104
		je  set3h
		jmp finisherr

	set3b:
		mov byte[calcbase], 2
		mov byte[myflag],   3
		jmp nextchar
	set3o:
		mov byte[calcbase], 8
		mov byte[myflag],   3
		jmp nextchar

	set3h:
		mov byte[calcbase], 16
		mov byte[myflag],   3
		jmp nextchar

	solve3:
		cmp bl, 32
		je  nextchar
		jmp finisherr


	set4:
		mov byte[myflag], 4
		jmp nextchar

	solve4:
		cmp bl, 32
		je  nextchar
		jmp finisherr

	finisherr:
		mov byte[myflag], 5
		jmp finished

	finished:
		ret

solve_num: ;处理num前导 0
	push eax
	push ecx
	mov  eax, num
	mov  ecx, num

	solve_num_loop1:
		cmp byte[eax], 48
		jne solve_num_loop2
		inc eax
		jmp solve_num_loop1

	solve_num_loop2:
		;byte[ecx]=byte[eax]
		push eax
		mov  al,        byte[eax]
		mov  byte[ecx], al
		pop  eax

		cmp byte[ecx], 0
		jz  end_solve_num
		inc ecx
		inc eax
		jmp solve_num_loop2
	end_solve_num:
		pop ecx
		pop eax
		ret

check_num: ;判断数落在正确区间内

	call solve_num            ;去0
	mov  byte[ismaximum], 1
	mov  eax,             num

	mov bl, byte[eax]
	mov bh, 0          ;index
	cmp bl, 0
	jz  calc_res
	cmp bl, 49
	jne no_one
	inc eax
	inc bh
	jmp loop_check_num

	loop_check_num:

		cmp byte[eax], 0
		jz  end_loop_check_num1
		cmp byte[eax], 48
		jne no_zero
		inc eax
		inc bh
		jmp loop_check_num

	no_one:
		mov byte[ismaximum], 0
		inc eax
		inc bh
		jmp loop_check_num

	no_zero:
		mov byte[ismaximum], 0
		inc eax
		inc bh
		jmp loop_check_num

	end_loop_check_num1:
		cmp bh, 31
		jg  printerror
		cmp bh, 31
		je  end_loop_check_num2
		jmp calc_res

	end_loop_check_num2:
		cmp byte[ismaximum], 1
		je  calc_res
		jmp printerror
		


calc_res: ;获取答案，高精除法, 模拟长除法，edx作为余数寄存器，eax作为被除数寄存器，ebx作为num指针，ecx作为res指针
	mov ecx, res
	begin_calc_loop:
		call solve_num
		; 判断被除数为0，0则结束
		mov  eax,       0
		mov  edx,       0
		mov  ebx,       num
		cmp  byte[ebx], 0
		jne  calc_loop      ;若被除数不为0,则calc_loop模拟长除法
		mov  byte[ecx], 0
		jmp  solveres

	calc_loop:
		cmp   byte[ebx], 0
		jz    end_calc_loop             ;遍历完毕，结束循环
		mov   eax,       edx
		imul  eax,       10
		push  ebx
		movzx ebx,       byte[ebx]
		add   eax,       ebx
		sub   eax,       48
		movzx ebx,       byte[calcbase]
		div   ebx
		pop   ebx
		mov   byte[ebx], al
		add   byte[ebx], 48
		inc   ebx
		jmp   calc_loop

	end_calc_loop:
		call get_digit
		mov  byte[ecx], dl
		inc  ecx
		jmp  begin_calc_loop


solveres: ;翻转并去除前导0
	mov eax, res
	mov ecx, res
	loop_reverse_begin:
		cmp byte[ecx], 0
		jz  loop_reverse_then
		inc ecx
		jmp loop_reverse_begin
	loop_reverse_then:
		dec ecx
	loop_reverse:
		cmp eax,       ecx
		jnl end_loop_reverse
		mov bh,        byte[eax]
		mov bl,        byte[ecx]
		mov byte[eax], bl
		mov byte[ecx], bh
		inc eax
		dec ecx
		jmp loop_reverse

	end_loop_reverse:
		mov eax, res
		mov ecx, res
	loop_nozero1:
		cmp byte[ecx], 48
		jne loop_nozero2
		inc ecx
		jmp loop_nozero1
	loop_nozero2:
		mov bl,        byte[ecx]
		mov byte[eax], bl
		cmp byte[ecx], 0
		jz  work_nozero1
		inc ecx
		inc eax
		jmp loop_nozero2
	work_nozero1:
		mov eax,       res
		cmp byte[eax], 0
		jz  work_nozero2
		jmp end_solveres
	work_nozero2:
		mov byte[eax], 48
		inc eax
		mov byte[eax], 0
	end_solveres:
		jmp printres

printres:
	push eax
	cmp  byte[calcbase], 2
	jz   output0b
	cmp  byte[calcbase], 8
	jz   output0o
	cmp  byte[calcbase], 16
	jz   output0x


	outputres:
	mov  eax, res
	call strprintln
	pop  eax
	jmp  _start

output0b:
	mov  eax, msg0b
	call strprint
	jmp  outputres
output0o:
	mov  eax, msg0o
	call strprint
	jmp  outputres

output0x:
	mov  eax, msg0x
	call strprint
	jmp  outputres



get_digit: ;获取数字(edx)对应字符
	cmp edx, 10
	jl  isdigit
	sub edx, 10
	add edx, 97
	ret
	isdigit:
		add edx, 48
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

read:
	push eax
	;mov  eax, msginput
	;call strprint
	push ebx
	push ecx
	push edx
	mov  edx, 255
	mov  ecx, input
	mov  ebx, 0
	mov  eax, 3
	int  80h
	pop  edx
	pop  ecx
	pop  ebx
	pop  eax
	ret

charprint: ;输出字符
	push edx
	push ecx
	push ebx
	mov  ecx, eax
	mov  edx, 1
	mov  ebx, 1
	mov  eax, 4
	int  80h
	pop  ebx
	pop  ecx
	pop  edx
	ret


strprint: ;输出字串
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

strprintln: ;输出字串带换行
	call strprint
	push eax
	mov  eax, 0ah
	push eax
	mov  eax, esp
	call strprint
	pop  eax
	pop  eax
	ret

printerror:
	push eax
	mov  eax, msgerror
	call strprintln
	pop  eax
	jmp  _start

quit: ;退出
	mov ebx, 0 ;正常退出
	mov eax, 1
	int 80h    ;调用 SYS_EXIT，正常退出
	ret

printnum:
	push eax
	mov  eax, num
	call strprintln
	pop  eax
	ret
```



##### 2.2 运行截图

![image-20240401153801800](https://raw.githubusercontent.com/mlger/Pict/main/newPath/image-20240401153801800.png)

#### 3. 实验问题

1. 8086 有哪 5 类寄存器？ 请分别举例说明其作⽤。

   1. **通用寄存器**：
      - 通用寄存器用于存储数据和执行算术和逻辑操作。
      - 例如：AX、BX、CX、DX 寄存器。其中，AX 可用于存放运算结果，BX 可用于存放内存地址，CX 可用于循环计数，DX 则可用于 I/O 操作。
   2. **段寄存器**：
      - 段寄存器用于存储段的起始地址，是实现内存分段的关键。
      - 例如：CS、DS、SS、ES 寄存器。CS 用于存放代码段的起始地址，DS 存放数据段的起始地址，SS 存放堆栈段的起始地址，ES 则是额外的数据段寄存器，用于一些特定的操作。
   3. **指令指针寄存器**：
      - 指令指针寄存器存储了 CPU 正在执行的指令的地址。
      - 例如：IP 寄存器，它存放着当前指令的偏移地址。
   4. **标志寄存器**：
      - 标志寄存器用于存储 CPU 执行指令后产生的标志位，例如零标志、进位标志等。
      - 例如：FLAGS 寄存器，包含了 AF（辅助进位标志）、ZF（零标志）、SF（符号标志）等。
   5. **控制寄存器**：
      - 控制寄存器用于控制 CPU 的工作模式和行为。
      - 例如：CS（代码段寄存器）、DS（数据段寄存器）、SS（堆栈段寄存器）等，这些寄存器可以用来改变 CPU 的工作状态，例如切换到保护模式等。

   这些寄存器的作用使得 CPU 能够执行各种不同的指令，并对数据进行处理、存储和传输。

2. 有哪些段寄存器， 它们的作⽤是什么？

   ​	对应用程序来说，主要设计3类段：存放程序中指令代码的代码段（Code Segment）、存放当前运行程序所用数据的数据段（Data Segment）和指明程序使用的堆栈区域的堆栈段（Stack Segment）。
   ​	为了表明段在主存中的位置，8086设计有4个16位段寄存器：代码段寄存器CS、堆栈段寄存器SS、数据段寄存器DS和附加段寄存器ES（Extra Segment）。其中，附加段也是用于存放数据的数据段，专为处理数据串设计的串指令必须使用附加段作为其目的操作数的存放区域。

3. 什么是寻址？ 8086 有哪些寻址⽅式？

   寻址是指找到操作数的地址(从而能够取出操作数)。

   8086的寻址方式

   - 立即寻址、直接寻址
   - 寄存器寻址、寄存器间接寻址、寄存器相对寻址
   - 基址加变址、相对基址加变址

4. 主程序与⼦程序之间如何传递参数

   - 利用寄存器传递参数
   - 缺点：能传递的参数有限，因为寄存器有限
   - 利用约定的地址传递参数
   - 利用堆栈传递参数（常用）

5. 解释 boot.asm ⽂件中 org 07c00h 的作⽤。 如果去掉这⼀句， 整个程序应该怎么修改？

   ​	告诉汇编器，当前这段代码会放在 07c00h处。所以，如果之后遇到需要绝对寻址的指令，那么绝对地址就是07c00h加上相对地址。

   ​	如果去掉 org 07c00h，需要修改程序中的所有跳转、访存和地址相关的指令，确保它们正确地指向引导扇区程序在内存中的新位置。同时，还需要确保程序的大小不超过 512 字节，并且适当调整程序入口点。这样 BIOS 才能正确加载引导扇区程序并执行。

6. 解释 int 10h 的功能。

   中断调用 10H 功能，用于执行文本和图形模式下的视频显示操作。

7. 解释 boot.asm ⽂件中 times 510-($-$$) db 0 的作⽤。

   零填充使文件大小达到 510 字节，以确保引导扇区的大小为 512 字节。

   - `times` 是一个汇编伪操作，它用于重复执行一段代码或填充一定数量的数据。
   - `510-($-$$)` 表示需要填充的字节数，其中 `$` 表示当前位置的地址。因此，`$-$$` 表示当前位置距离文件开头的偏移量。`510-($-$$)` 计算了当前位置到文件开头的偏移量，然后将其与 510 比较，以确定需要填充多少字节。
   - `db 0` 表示以字节为单位将零填充到指定的位置。

8. 解释 bochsrc 中各参数的含义。

   1. `megs:32`：
      - 这指定了虚拟机或仿真器应该分配给操作系统或应用程序的内存量。在这种情况下，32 表示分配了 32MB 的内存。这可以用来模拟具有指定内存量的计算机环境。
   2. `display_library: sdl2`：
      - 这指定了用于显示的库或驱动程序。在这里，`sdl2` 可能是指 Simple DirectMedia Layer（简称SDL）的第二个版本，它是一个跨平台的多媒体库，用于在不同的平台上处理音频、视频和输入。使用 SDL2 可能是为了在模拟器或虚拟机中提供跨平台的图形支持。
   3. `floppya: 1_44=a.img, status=inserted`：
      - 这定义了一个虚拟的软盘驱动器，指定了软盘的类型和镜像文件的位置。在这种情况下，`1_44` 表示一个 1.44MB 的软盘，`a.img` 是镜像文件的路径。`status=inserted` 表示该软盘已插入到虚拟软盘驱动器中。
   4. `boot: floppy`：
      - 这告诉虚拟机或仿真器在启动时应该从软盘驱动器引导。也就是说，操作系统或引导程序应该从软盘镜像 (`a.img`) 中加载。
   5. `romimage: file=$BXSHARE/BIOS-bochs-legacy` 这一行指定了一个 ROM 镜像文件的路径，路径为 `$BXSHARE/BIOS-bochs-legacy`。

9. boot.bin 应该放在软盘的哪⼀个扇区？ 为什么？

   `boot.bin` 需要被放置在软盘的第一个扇区，以便 BIOS 能够正确加载并执行其中的引导程序。

10. 为什么不让 Boot 程序直接加载内核， ⽽需要先加载 Loader 再加载内核？

    在一些操作系统设计中，引导程序（Bootloader）和加载程序（Loader）的分离是有一些好处的：

    1. **复杂度管理**：引导程序负责在计算机启动时加载最基本的系统组件，如引导扇区、文件系统等，而加载程序则负责更复杂的任务，例如从文件系统中加载内核、初始化内存、设置环境等。将这些任务分开可以更好地管理复杂度，使得引导程序更加简洁、可靠。
    2. **灵活性**：通过将加载程序与引导程序分开，可以更灵活地设计系统。例如，加载程序可以支持从不同类型的文件系统中加载内核，而引导程序则可以专注于在引导阶段完成最基本的硬件初始化和引导过程。
    3. **模块化设计**：将引导过程分成多个阶段可以实现系统的模块化设计。这样，如果需要更改加载过程中的某一部分，只需要修改加载程序而不影响引导程序，从而提高了系统的可维护性和扩展性。
    4. **错误处理**：通过引导程序和加载程序的分离，可以更好地处理错误情况。例如，如果加载程序在加载内核时出现错误，可以有机会显示错误消息或采取其他适当的处理措施，而不会影响到引导程序的功能。

    总的来说，通过引导程序和加载程序的分离，可以更好地管理复杂度、提高灵活性、实现模块化设计并改善错误处理，从而提高操作系统的可靠性和可维护性。

11. Loader 的作⽤有哪些？

    加载程序（Loader）在操作系统启动过程中扮演着重要的角色，其主要作用包括：

    1. **加载内核**：加载程序负责从存储介质（如硬盘、软盘、网络等）中读取操作系统内核的镜像文件，并将其加载到内存中。加载内核是操作系统启动过程中最关键的任务之一。
    2. **初始化内存**：加载程序可能需要在加载内核之前对系统内存进行初始化。这包括设置内存分页、建立内存映射表、分配内存空间等操作，以确保内核能够正常运行。
    3. **设置环境**：加载程序负责设置操作系统运行所需的执行环境。这包括设置堆栈、参数传递、初始化全局变量等操作，以确保内核能够正确执行。
    4. **启动内核**：加载程序加载完内核后，将控制权转移给内核，启动操作系统的正常运行。这通常是通过跳转到内核的入口点来实现的。
    5. **错误处理**：加载程序可能需要处理加载过程中出现的错误情况。这包括检测加载错误、显示错误信息、尝试恢复等操作，以确保系统能够正确启动或提供错误信息以供用户参考。

    总的来说，加载程序负责将操作系统内核加载到内存中，并为其提供必要的执行环境，从而启动操作系统的正常运行。加载程序在操作系统启动过程中起着至关重要的作用。

12. Kernel 的作⽤有哪些？

    操作系统内核（Kernel）是操作系统的核心部分，负责管理计算机的硬件资源并提供各种系统服务。其主要作用包括：

    1. **硬件管理**：内核管理计算机的硬件资源，包括处理器、内存、输入输出设备等。它负责分配和管理这些资源，以确保它们能够被应用程序有效地使用。
    2. **进程管理**：内核负责管理系统中运行的进程。它分配处理器时间片给不同的进程，并调度它们的执行顺序。此外，内核还负责进程的创建、终止、暂停和恢复等操作。
    3. **内存管理**：内核管理系统的内存资源，包括物理内存和虚拟内存。它负责将进程的内存空间映射到物理内存中，并处理内存的分配、回收和页面置换等操作。
    4. **文件系统**：内核提供文件系统服务，允许应用程序访问和管理文件和目录。它负责文件的创建、打开、读写、关闭等操作，并管理文件系统的存储空间。
    5. **设备驱动程序**：内核包含设备驱动程序，允许应用程序通过标准接口与硬件设备进行通信。它负责处理设备的初始化、数据传输、中断处理等操作。
    6. **系统调用接口**：内核提供系统调用接口，允许应用程序向内核请求系统服务。这些系统调用包括文件操作、进程管理、网络通信等，为应用程序提供了访问操作系统功能的途径。
    7. **安全性和权限管理**：内核负责实施安全策略和权限管理，保护系统和用户数据免受未经授权的访问和恶意软件的攻击。

    总的来说，内核是操作系统的核心部分，负责管理系统的硬件资源、提供系统服务和接口，并保证系统的安全性和稳定性。它为应用程序提供了一个统一的接口，使它们能够在计算机系统上运行并与硬件设备进行通信。

