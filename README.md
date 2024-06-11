#### 一、添加系统调用

##### 1. 系统调用与用户调用声明

```c
// include/proto.h
/* 以下是系统调用相关 */

// Lg: 系统级
/* proc.c */
PUBLIC  int     sys_get_ticks();        /* sys_call */
PUBLIC	void 	sys_sleep_ms(int milli_sec);
PUBLIC	void 	sys_print_str(char* buf, int col);
PUBLIC	void 	p_process(SEMAPHORE* sem);
PUBLIC	void 	v_process(SEMAPHORE* sem);
PUBLIC	void 	sys_work(int milli_sec);

// Lg: 用户级
/* syscall.asm */
PUBLIC  void    sys_call();             /* int_handler */
PUBLIC  int     get_ticks();
PUBLIC	void 	sleep_ms(int milli_sec);
PUBLIC	void 	print_str(char* buf, int col);
PUBLIC	void 	P(SEMAPHORE* sem);
PUBLIC	void 	V(SEMAPHORE* sem);
PUBLIC	void 	work(int milli_sec);
```

##### 2. 添加系统调用表

```c
// kernel/global.c
// Lg: 添加系统调用
PUBLIC system_call sys_call_table[NR_SYS_CALL] = {
    sys_get_ticks,
	sys_sleep_ms,
	sys_print_str,
	p_process,
	v_process,
	sys_work
};
```

##### 3. 系统调用实现

代码于 `kernel/proc.c`  中

##### 4. 用户调用

代码主要于 `kernel/syscall.asm`。

很多同学在这里出现了 Page Fault 的情况，绝大部分是没有**保护好现场**。

举一个例子，双参数的打印调用：

```
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
```

`push ecx`, `push ebx`, `pop ebx`, `pop ecx` 起保护现场作用且 `push` 与 `pop` 需对称。

**为什么第一、二个参数分别是 `esp + 12` 和 `esp + 16` 呢？**

![fe647bd48fec9be273c35129ed780ba4](https://raw.githubusercontent.com/mlger/Pict/main/newPath/fe647bd48fec9be273c35129ed780ba4.png)

#### 二、读者写者问题

##### 1. 前言

信号量数据结构：

```c
typedef struct s_sema {
    int value;
    int head;
    int tail;
    PROCESS* process_list[NR_PROCS];  // 进程队列
} SEMAPHORE;
```

其中进程队列是单向循环队列，长度刚好能够容纳所有进程。

资源准备：

```c
// Lg: 添加信号量
PUBLIC  SEMAPHORE read_write_mutex = {1, 0, 0};	// 读写互斥信号量
PUBLIC  SEMAPHORE writer_cnt_mutex = {1, 0, 0};	// 访问写者数量
PUBLIC  SEMAPHORE reader_cnt_mutex = {1, 0, 0};	// 访问读者数量
PUBLIC  SEMAPHORE queue = {1, 0, 0};	// 队列信号量, 用于控制读写顺序
PUBLIC  SEMAPHORE reader_mutex = {MAX_READERS, 0, 0};	// 读者数量互斥信号量
```

由于需要解决进程饿死问题，所以此任务实际上**实现了读者优先、写者优先、读写公平三种策略**。

##### 2. 读者优先

```c
/*======================================================================*
                               读者优先
 *======================================================================*/
void read_reader_first(int slices) {
	P(&queue);
	P(&reader_mutex);  // 读者上限
    P(&reader_cnt_mutex);  // 调取读者数量
    if (readers == 0) {
        P(&read_write_mutex);  // 读写互斥
    }
    readers++;
    V(&reader_cnt_mutex);  // 释放读者数量
	V(&queue);

    // 读操作
    read_process(slices);

    P(&reader_cnt_mutex);  // 调取读者数量
    readers--;
    if (readers == 0) {
        V(&read_write_mutex);  // 读写互斥解锁
    }
    V(&reader_cnt_mutex);  // 释放读者数量
    V(&reader_mutex);      // 读者上限
}

void write_reader_first(int slices) {
	P(&queue);
	V(&queue);
    P(&read_write_mutex);  // 读写互斥

    // 写操作
    write_process(slices);

    V(&read_write_mutex);  // 读写互斥解锁

}
```

限制 1 位读者同时读，读完休息 0, 1, 2 时间片：

![image-20240611165631068](https://raw.githubusercontent.com/mlger/Pict/main/newPath/image-20240611165631068.png)

限制 2 位读者同时读，读完休息 0, 1, 2 时间片：

![image-20240611165829755](https://raw.githubusercontent.com/mlger/Pict/main/newPath/image-20240611165829755.png)

限制 3 位读者同时读，读完休息 0, 1, 2 时间片：

![image-20240611165937274](https://raw.githubusercontent.com/mlger/Pict/main/newPath/image-20240611165937274.png)

##### 3. 写者优先

```c
/*======================================================================*
                               写者优先
 *======================================================================*/
void read_writer_first(int slices) {
    P(&reader_mutex);  // 读者上限
    P(&queue);
    P(&reader_cnt_mutex);  // 调取读者数量
    if (readers == 0) {
        P(&read_write_mutex);  // 读写互斥
    }
    readers++;
    V(&reader_cnt_mutex);  // 释放读者数量
    V(&queue);

    // 读操作
    read_process(slices);

    P(&reader_cnt_mutex);  // 调取读者数量
    readers--;
    if (readers == 0) {
        V(&read_write_mutex);  // 读写互斥解锁
    }
    V(&reader_cnt_mutex);  // 释放读者数量
    V(&reader_mutex);      // 读者上限
}

void write_writer_first(int slices) {
    P(&writer_cnt_mutex);  // 调取写者数量
    if (writers == 0) {
        P(&queue);
    }
    writers++;
    V(&writer_cnt_mutex);  // 释放写者数量

    // 写操作
    P(&read_write_mutex);  // 读写互斥
    write_process(slices);
    V(&read_write_mutex);  // 读写互斥解锁

    P(&writer_cnt_mutex);  // 调取写者数量
    writers--;
    if (writers == 0) {
        V(&queue);
    }
    V(&writer_cnt_mutex);  // 释放写者数量
}
```

限制 1 位读者同时读，读完休息 0, 1, 2 时间片：

![image-20240611170320786](https://raw.githubusercontent.com/mlger/Pict/main/newPath/image-20240611170320786.png)

限制 2 位读者同时读，读完休息 0, 2, 4 时间片：

![image-20240611171431289](https://raw.githubusercontent.com/mlger/Pict/main/newPath/image-20240611171431289.png)

限制 3 位读者同时读，读完休息 0, 2, 4 时间片：

![image-20240611171527657](https://raw.githubusercontent.com/mlger/Pict/main/newPath/image-20240611171527657.png)

##### 4. 公平读写

```c
void read_fair(int slices) {
    P(&queue);
    P(&reader_mutex);      // 读者上限
    P(&reader_cnt_mutex);  // 调取读者数量
    if (readers == 0) {
        P(&read_write_mutex);  // 读写互斥
    }
    readers++;
    V(&reader_cnt_mutex);  // 释放读者数量
    V(&queue);

    // 读操作
    read_process(slices);

    P(&reader_cnt_mutex);  // 调取读者数量
    readers--;
    if (readers == 0) {
        V(&read_write_mutex);  // 读写互斥解锁
    }
    V(&reader_cnt_mutex);  // 释放读者数量
    V(&reader_mutex);      // 读者上限
}

void write_fair(int slices) {
    P(&queue);
    P(&read_write_mutex);  // 读写互斥

    // 写操作
    write_process(slices);

    V(&read_write_mutex);  // 读写互斥解锁
    V(&queue);
}
```

限制 1, 2, 3 位读者同时读，读完休息 2 时间片：

![image-20240611171730821](https://raw.githubusercontent.com/mlger/Pict/main/newPath/image-20240611171730821.png)

##### 5. 细节问题

目前代码已跑通所有期望输出，但逻辑上仍存在一些问题。下面给出错误输出的原版本读者优先代码。

```c
void read_reader_first(int slices) {
	P(&reader_mutex);  // 读者上限  --------1
    P(&reader_cnt_mutex);  // 调取读者数量
    if (readers == 0) {
        P(&read_write_mutex);  // 读写互斥
    }
    readers++;
    V(&reader_cnt_mutex);  // 释放读者数量

    // 读操作
    read_process(slices);

    P(&reader_cnt_mutex);  // 调取读者数量
    readers--;
    if (readers == 0) {
        V(&read_write_mutex);  // 读写互斥解锁 ------2
    }
    V(&reader_cnt_mutex);  // 释放读者数量 
    V(&reader_mutex);      // 读者上限     --------3
}

void write_reader_first(int slices) {
    P(&read_write_mutex);  // 读写互斥 ----------4

    // 写操作
    write_process(slices);
    
    V(&read_write_mutex);  // 读写互斥解锁

}
```

假设读者上限为 1，读者优先，初始进程队列 R1, R2, W1。

1. 在 R1执行完毕后，先执行代码 2，释放读者互斥锁。
2. 由于 R2 受代码 1 影响，此时读者上限仍未释放，但 W1 从读写互斥锁释放出来，优先执行 W1。
3. 这样，即使是读者优先，且队列是读-读-写，但仍造成了读-写-读的反常情况。

#### 三、

#### 四、参考资料

1. [进程同步经典问题之读者写者问题 - 知乎](https://zhuanlan.zhihu.com/p/538487720)
