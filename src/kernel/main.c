
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                            main.c
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                                                    Forrest Yu, 2005
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

#include "type.h"
#include "const.h"
#include "protect.h"
#include "string.h"
#include "proc.h"
#include "tty.h"
#include "console.h"
#include "global.h"
#include "proto.h"

int sleep_t;

PRIVATE void init_semaphore(SEMAPHORE* sem, int value);
void print_semaphore_info(SEMAPHORE* sem);
/*======================================================================*
                            init_tasks
 *======================================================================*/
PRIVATE void init_tasks() {
    init_screen(tty_table);
    clean_screen(console_table);

    // 初始化变量
    k_reenter = 0;  // 重入中断数
    ticks = 0;      // 时钟中断数

    sleep_t = 1;  // 执行完，睡眠时间

    p_proc_ready = proc_table;
}

/*======================================================================*
                            kernel_main
 *======================================================================*/
PUBLIC int kernel_main() {
    disp_str("-----\"kernel_main\" begins-----\n");

    TASK* p_task = task_table;
    PROCESS* p_proc = proc_table;
    char* p_task_stack = task_stack + STACK_SIZE_TOTAL;
    u16 selector_ldt = SELECTOR_LDT_FIRST;
    int i;
    u8 privilege;
    u8 rpl;
    int eflags;
    for (i = 0; i < NR_TASKS + NR_PROCS; i++) {
        if (i < NR_TASKS) { /* 任务 */
            p_task = task_table + i;
            privilege = PRIVILEGE_TASK;
            rpl = RPL_TASK;
            eflags = 0x1202; /* IF=1, IOPL=1, bit 2 is always 1 */
        } else {             /* 用户进程 */
            p_task = user_proc_table + (i - NR_TASKS);
            privilege = PRIVILEGE_USER;
            rpl = RPL_USER;
            eflags = 0x202; /* IF=1, bit 2 is always 1 */
        }

        strcpy(p_proc->p_name, p_task->name);  // name of the process
        p_proc->pid = i;                       // pid
        p_proc->wake_tick = 0;                 // 唤醒时间
        p_proc->state = STRUN;                 // 进程状态
        p_proc->gen_cost = 0;                  // 生成消耗

        p_proc->ldt_sel = selector_ldt;

        memcpy(&p_proc->ldts[0], &gdt[SELECTOR_KERNEL_CS >> 3],
               sizeof(DESCRIPTOR));
        p_proc->ldts[0].attr1 = DA_C | privilege << 5;
        memcpy(&p_proc->ldts[1], &gdt[SELECTOR_KERNEL_DS >> 3],
               sizeof(DESCRIPTOR));
        p_proc->ldts[1].attr1 = DA_DRW | privilege << 5;
        p_proc->regs.cs = (0 & SA_RPL_MASK & SA_TI_MASK) | SA_TIL | rpl;
        p_proc->regs.ds = (8 & SA_RPL_MASK & SA_TI_MASK) | SA_TIL | rpl;
        p_proc->regs.es = (8 & SA_RPL_MASK & SA_TI_MASK) | SA_TIL | rpl;
        p_proc->regs.fs = (8 & SA_RPL_MASK & SA_TI_MASK) | SA_TIL | rpl;
        p_proc->regs.ss = (8 & SA_RPL_MASK & SA_TI_MASK) | SA_TIL | rpl;
        p_proc->regs.gs = (SELECTOR_KERNEL_GS & SA_RPL_MASK) | rpl;

        p_proc->regs.eip = (u32)p_task->initial_eip;
        p_proc->regs.esp = (u32)p_task_stack;
        p_proc->regs.eflags = eflags;

        p_task_stack -= p_task->stacksize;
        p_proc++;
        p_task++;
        selector_ldt += 1 << 3;
    }

    init_tasks();
    init_clock();
    init_keyboard();

    restart();

    while (1) {
    }
}

// 函数声明
PRIVATE void write(int num, int col);
PRIVATE void writesp(int num, int col);
PRIVATE void writeln(int num, int col);

/*======================================================================*
                               进程
 *======================================================================*/
void Reporter() {
    int cnt = 0;
    while (1) {
        if (cnt < 20) {
            writesp(cnt, DEFAULT_CHAR_COLOR);
            if (cnt <= 0x0F)
                print_str(" ", DEFAULT_CHAR_COLOR);

            for (PROCESS* p = proc_table + NR_TASKS + 1;
                 p < proc_table + NR_TASKS + NR_PROCS; p++) {
                if (p->state == STBLOCK) {
                    print_str(" X ", RED);
                } else if (p->state == STSLEEP) {
                    print_str(" Z ", BLUE);
                } else {
                    print_str(" O ", GREEN);
                }
            }
            print_str("\n", DEFAULT_CHAR_COLOR);
        }
        ++cnt;
        sleep_ms(TIME_SLICE);
    }
}

void P1() {
    while (1) {
        sleep_ms(sleep_t * TIME_SLICE);
    }
}

void P2() {
    while (1) {
        sleep_ms(sleep_t * TIME_SLICE);
    }
}

void C1() {
    while (1) {
        sleep_ms(sleep_t * TIME_SLICE);
    }
}

void C2() {
    while (1) {
        sleep_ms(sleep_t * TIME_SLICE);
    }
}

void C3() {
    while (1) {
        sleep_ms(sleep_t * TIME_SLICE);
    }
}

/*======================================================================*
                               生产消耗
 *======================================================================*/
PRIVATE void gen_cost(PROCESS* p) {
    ++p->gen_cost;
}

/*======================================================================*
                                                           打印数字
 *======================================================================*/
PRIVATE void write(int num, int col) {
    printf_col(col, "%x", num);
}

PRIVATE void writesp(int num, int col) {
    printf_col(col, "%x ", num);
}

PRIVATE void writeln(int num, int col) {
    printf_col(col, "%x\n", num);
}