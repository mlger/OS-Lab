
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
                           辅助函数
 *======================================================================*/
PUBLIC void block(SEMAPHORE* sem) {
    p_proc_ready->state = STBLOCK;
    p_proc_ready->wake_tick = 0x7fffffff;
    sem->process_list[sem->tail] = p_proc_ready;
    sem->tail = (sem->tail + 1) % NR_PROCS;
    schedule();
}

PUBLIC void wakeup(SEMAPHORE* sem) {
    PROCESS* p = sem->process_list[sem->head];
    sem->head = (sem->head + 1) % NR_PROCS;
    p->wake_tick = 0;
}

PRIVATE void setState(int ticks, int state) {
    p_proc_ready->wake_tick = get_ticks() + ticks + 1;
    p_proc_ready->state = state;
}
/*======================================================================*
                              schedule
 *======================================================================*/
PUBLIC void schedule() {
    PROCESS* p;
    int current_tick = get_ticks();

    while (1) {
        p_proc_ready++;
        if (p_proc_ready >= proc_table + NR_TASKS + NR_PROCS) {
            p_proc_ready = proc_table;
        }
        if (p_proc_ready->state == STRUN &&
            p_proc_ready->wake_tick <= current_tick) {
            break;  // 找到进程
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
    // disp_str("sys_sleep_ms: ");
    // disp_int(milli_sec);

    int ticks = milli_sec * HZ / 1000;
    setState(ticks, STSLEEP);
    schedule();

    // disp_str("       exit sys_sleep_ms\n");
}

PUBLIC void sys_print_str(char* buf, int col) {
    CONSOLE* p_con = &console_table[0];
    // col
    set_color(p_con, col);

    // print
    char* p = buf;
    while (*p) {
        out_char(p_con, *p++);
    }

    // recover col
    set_color(p_con, DEFAULT_CHAR_COLOR);
}

PUBLIC void p_process(SEMAPHORE* sem) {
    // disp_str("p_process: ");
    // print_semaphore_info(sem);

    disable_int();
    sem->value--;
    if (sem->value < 0) {
        block(sem);
    }
    enable_int();
}

PUBLIC void v_process(SEMAPHORE* sem) {
    // disp_str("v_process: ");
    // print_semaphore_info(sem);

    disable_int();
    sem->value++;
    if (sem->value <= 0) {
        wakeup(sem);
    }
    enable_int();
}

PUBLIC void sys_work(int milli_sec) {
    // disp_str("sys_work: ");
    // disp_int(milli_sec);

    int ticks = milli_sec * HZ / 1000;
    setState(ticks, STRUN);
    schedule();
    // disp_str("       exit sys_work\n");
}
