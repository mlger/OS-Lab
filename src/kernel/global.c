
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
                                         {P1, STACK_SIZE_P1, "P1"},
                                         {P2, STACK_SIZE_P2, "P2"},
										 {C1, STACK_SIZE_C1, "C1"},
										 {C2, STACK_SIZE_C2, "C2"},
										 {C3, STACK_SIZE_C3, "C3"}};

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
PUBLIC SEMAPHORE sem_empty = {0, 0, 0}; // 空缓冲区
PUBLIC SEMAPHORE sem_full1 = {0, 0, 0};  // 满缓冲区1
PUBLIC SEMAPHORE sem_full2 = {0, 0, 0};  // 满缓冲区2
//PUBLIC SEMAPHORE sem_mutex = {1, 0, 0};  // 互斥信号量


