
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                            global.c
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                                                    Forrest Yu, 2005
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

#define GLOBAL_VARIABLES_HERE

#include "type.h"
#include "const.h"
#include "protect.h"
#include "tty.h"
#include "console.h"
#include "proc.h"
#include "global.h"
#include "proto.h"

PUBLIC PROCESS proc_table[NR_TASKS + NR_PROCS];

PUBLIC TASK task_table[NR_TASKS] = {{task_tty, STACK_SIZE_TTY, "tty"}};

PUBLIC TASK user_proc_table[NR_PROCS] = {{Reporter, STACK_SIZE_REPORTER, "Reporter"},
                                         {R1, STACK_SIZE_R1, "R1"},
                                         {R2, STACK_SIZE_R2, "R2"},
										 {R3, STACK_SIZE_R3, "R3"},
										 {W1, STACK_SIZE_W1, "W1"},
										 {W2, STACK_SIZE_W2, "W2"}};

PUBLIC char task_stack[STACK_SIZE_TOTAL];

PUBLIC TTY tty_table[NR_CONSOLES];
PUBLIC CONSOLE console_table[NR_CONSOLES];

PUBLIC irq_handler irq_table[NR_IRQ];

// Lg: 添加系统调用
PUBLIC system_call sys_call_table[NR_SYS_CALL] = {
    sys_get_ticks,
	sys_sleep_ms,
	sys_print_str,
	p_process,
	v_process,
	sys_work
};

// Lg: 添加信号量
PUBLIC  SEMAPHORE read_write_mutex = {1, 0, 0};	// 读写互斥信号量
PUBLIC  SEMAPHORE writer_cnt_mutex = {1, 0, 0};	// 访问写者数量
PUBLIC  SEMAPHORE reader_cnt_mutex = {1, 0, 0};	// 访问读者数量
PUBLIC  SEMAPHORE queue = {1, 0, 0};	// 队列信号量, 用于控制读写顺序
PUBLIC  SEMAPHORE reader_mutex = {MAX_READERS, 0, 0};	// 读者数量互斥信号量
