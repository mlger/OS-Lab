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

#### 四、代码测试

##### 1. 镜像结构

![image-20240422201023930](https://raw.githubusercontent.com/mlger/Pict/main/newPath/image-20240422201023930.png)

##### 2. 标准指令测试

- 空输入

![image-20240422201128538](https://raw.githubusercontent.com/mlger/Pict/main/newPath/image-20240422201128538.png)

- ls 不带路径与参数

![image-20240422201210582](https://raw.githubusercontent.com/mlger/Pict/main/newPath/image-20240422201210582.png)

- ls 带路径 不带参数

![image-20240422201240796](https://raw.githubusercontent.com/mlger/Pict/main/newPath/image-20240422201240796.png)

- ls 不带路径 带参数

![image-20240422201301361](https://raw.githubusercontent.com/mlger/Pict/main/newPath/image-20240422201301361.png)

- ls 带路径 带参数

![image-20240422201318121](https://raw.githubusercontent.com/mlger/Pict/main/newPath/image-20240422201318121.png)

- 多重复参数剔除

![image-20240422201400066](https://raw.githubusercontent.com/mlger/Pict/main/newPath/image-20240422201400066.png)

- cat

![image-20240422201524205](https://raw.githubusercontent.com/mlger/Pict/main/newPath/image-20240422201524205.png)

##### 3. 错误情形测试

- cat 目录

![image-20240422201558375](https://raw.githubusercontent.com/mlger/Pict/main/newPath/image-20240422201558375.png)

- 目标文件/目录不存在（返回最长匹配路径）

![image-20240422201714720](https://raw.githubusercontent.com/mlger/Pict/main/newPath/image-20240422201714720.png)

![image-20240422201849542](https://raw.githubusercontent.com/mlger/Pict/main/newPath/image-20240422201849542.png)

- 参数不存在（实现过程中，对每个命令分配一个动态数组，其中保存可匹配的参数）

![image-20240422201905300](https://raw.githubusercontent.com/mlger/Pict/main/newPath/image-20240422201905300.png)

- 命令不存在

![image-20240422201941493](https://raw.githubusercontent.com/mlger/Pict/main/newPath/image-20240422201941493.png)
