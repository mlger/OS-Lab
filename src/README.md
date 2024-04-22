#### 一、项目结构

```bash
lg@ubuntu:~/Documents/OS/OS-Lab/src$ tree
.
├── a.img
├── mount
├── Makefile
├── FAT12-reader.cpp	# 主程序
├── myString.h	# 辅助库（自建字符串库）
└── print.asm	# 汇编输出
```

#### 二、实现功能

包括带参数的 ls、cat，支持小写文件名，支持多于 512 字节 cat。

#### 三、代码运行

首先配置 32 位编译环境

```bash
sudo apt install g++-multilib
```

编译汇编代码

````bash
nasm -f elf32 -o print.o print.asm
````

使用32位编译器编译和链接C++代码

```bash
g++ -m32 -o hello FAT12-reader.cpp print.o
```

运行程序

```bash
./hello
```

