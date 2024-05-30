
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

int strategy;
int sleep_t;
PUBLIC void print_status(PROCESS* p);
int cost_time_R1, cost_time_R2, cost_time_R3, cost_time_W1, cost_time_W2;

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
    readers = 0;    // 读者数量
    writers = 0;    // 写者数量

    cost_time_R1 = 2;  // R1 操作耗时
    cost_time_R2 = 3;  // R2 操作耗时
    cost_time_R3 = 3;  // R3 操作耗时
    cost_time_W1 = 3;  // W1 操作耗时
    cost_time_W2 = 4;  // W2 操作耗时

    strategy = 2;  // 读写策略, 0: 读者优先, 1: 写者优先, 2: 读写公平
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
PRIVATE void read_process(int silces);
PRIVATE void write_process(int silces);
PRIVATE void write(int num, int col);
PRIVATE void writesp(int num, int col);
PRIVATE void writeln(int num, int col);
void read_reader_first(int slices);
void write_reader_first(int slices);
void read_writer_first(int slices);
void write_writer_first(int slices);
void read_fair(int slices);
void write_fair(int slices);

read_f read_funcs[3] = {read_reader_first, read_writer_first, read_fair};
write_f write_funcs[3] = {write_reader_first, write_writer_first, write_fair};

/*======================================================================*
                               读者优先
 *======================================================================*/
void read_reader_first(int slices) {
    P(&reader_mutex);      // 读者上限
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
        V(&read_write_mutex);  // 读写互斥解锁
    }
    V(&reader_cnt_mutex);  // 释放读者数量
    V(&reader_mutex);      // 读者上限
}

void write_reader_first(int slices) {
    P(&read_write_mutex);  // 读写互斥

    // 写操作
    write_process(slices);

    V(&read_write_mutex);  // 读写互斥解锁
}

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

/*======================================================================*
                               读写公平
 *======================================================================*/
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

void R1() {
    while (1) {
        read_funcs[strategy](cost_time_R1);
        sleep_ms(sleep_t * TIME_SLICE);
    }
}

void R2() {
    while (1) {
        read_funcs[strategy](cost_time_R2);
        sleep_ms(sleep_t * TIME_SLICE);
    }
}

void R3() {
    while (1) {
        read_funcs[strategy](cost_time_R3);
        sleep_ms(sleep_t * TIME_SLICE);
    }
}

void W1() {
    while (1) {
        write_funcs[strategy](cost_time_W1);
        sleep_ms(sleep_t * TIME_SLICE);
    }
}

void W2() {
    while (1) {
        write_funcs[strategy](cost_time_W2);
        sleep_ms(sleep_t * TIME_SLICE);
    }
}

/*======================================================================*
                               读写操作
 *======================================================================*/
PRIVATE void read_process(int silces) {
    //printf("\n\nreadcost_time: %x\n\n", silces);
    work(silces * TIME_SLICE);  // 读耗时 silces 个时间片
}
PRIVATE void write_process(int silces) {
    //printf("\n\nwritecost_time: %x\n\n", silces);
    work(silces * TIME_SLICE);  // 写耗时 silces 个时间片
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