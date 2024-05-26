
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                               proc.c
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                                                    Forrest Yu, 2005
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

#include "type.h"
#include "const.h"
#include "protect.h"
#include "tty.h"
#include "console.h"
#include "string.h"
#include "proc.h"
#include "global.h"
#include "proto.h"

/*======================================================================*
                              schedule
 *======================================================================*/
PUBLIC void schedule() {
    PROCESS* p;
    int greatest_ticks = 0;

    while (!greatest_ticks) {
        for (p = proc_table; p < proc_table + NR_TASKS + NR_PROCS; p++) {
            if (p->issleep || p->isblocked)
                continue;  // Lg:睡眠或阻塞
            if (p->ticks > greatest_ticks) {
                greatest_ticks = p->ticks;
                p_proc_ready = p;
            }
        }

        if (!greatest_ticks) {
            for (p = proc_table; p < proc_table + NR_TASKS + NR_PROCS; p++) {
                if (p->issleep || p->isblocked) {  // Lg:睡眠或阻塞
                    continue;
                }
                p->ticks = p->priority;
            }
        }
    }
}

/*======================================================================*
                           sys_call
 *======================================================================*/
PUBLIC int sys_get_ticks() {
    return ticks;
}

PUBLIC void sys_sleep_ms(int milli_sec) {
    int ticks = milli_sec * HZ / 1000;
    p_proc_ready->issleep = 1;
    p_proc_ready->sleeping_ticks = ticks;
    schedule();
}

PUBLIC void sys_print_str(char* buf, int len) {
	CONSOLE* p_con = &console_table[0];
	for (int i = 0; i < len; i++) {
		out_char(p_con, buf[i]);
	}
}

PUBLIC void p_process(SEMAPHORE* sem) {
	disable_int();
	sem->value--;
	if (sem->value < 0) {
		block(sem);
	}
	enable_int();
}

PUBLIC void v_process(SEMAPHORE* sem) {
	disable_int();
	sem->value++;
	if (sem->value <= 0) {
		wakeup(sem);
	}
	enable_int();
}

/*======================================================================*
                           辅助函数
 *======================================================================*/
PUBLIC void block(SEMAPHORE* sem) {
	p_proc_ready->isblocked = 1;
	sem->process_list[sem->tail] = p_proc_ready;
	sem->tail = (sem->tail + 1) % NR_PROCS;
	schedule();
}
PUBLIC void wakeup(SEMAPHORE* sem) {
	PROCESS* p = sem->process_list[sem->head];
	sem->head = (sem->head + 1) % NR_PROCS;
	p->isblocked = 0;	
}