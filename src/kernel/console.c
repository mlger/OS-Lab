
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                              console.c
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                                                    Forrest Yu, 2005
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

/*
        回车键: 把光标移到第一列
        换行键: 把光标前进到下一行
*/

#include "type.h"
#include "const.h"
#include "protect.h"
#include "string.h"
#include "proc.h"
#include "tty.h"
#include "console.h"
#include "global.h"
#include "keyboard.h"
#include "proto.h"

PRIVATE void set_cursor(unsigned int position);
PRIVATE void set_video_start_addr(u32 addr);
PRIVATE void flush(CONSOLE* p_con);

/*======================================================================*
                           init_screen
 *======================================================================*/
PUBLIC void init_screen(TTY* p_tty) {
    int nr_tty = p_tty - tty_table;
    p_tty->p_console = console_table + nr_tty;

    int v_mem_size = V_MEM_SIZE >> 1; /* 显存总大小 (in WORD) */

    int con_v_mem_size = v_mem_size / NR_CONSOLES;
    p_tty->p_console->original_addr = nr_tty * con_v_mem_size;
    p_tty->p_console->v_mem_limit = con_v_mem_size;
    p_tty->p_console->current_start_addr = p_tty->p_console->original_addr;

    /* 默认光标位置在最开始处 */
    p_tty->p_console->cursor = p_tty->p_console->original_addr;

    if (nr_tty == 0) {
        /* 第一个控制台沿用原来的光标位置 */
        p_tty->p_console->cursor = disp_pos / 2;
        disp_pos = 0;
    } else {
        out_char(p_tty->p_console, nr_tty + '0');
        out_char(p_tty->p_console, '#');
    }

    set_cursor(p_tty->p_console->cursor);
}

/*======================================================================*
                           is_current_console
*======================================================================*/
PUBLIC int is_current_console(CONSOLE* p_con) {
    return (p_con == &console_table[nr_current_console]);
}

/*======================================================================*
                           out_char
 *======================================================================*/
PUBLIC void out_char(CONSOLE* p_con, char ch) {
    u8* p_vmem = (u8*)(V_MEM_BASE + p_con->cursor * 2);
    //INPUT_RECORD record;
    if (mode == 0) {  // Lg: mode0 编辑模式
        switch (ch) {
            case '\n':  // 行内剩余空格标记颜色
                if (p_con->cursor <
                    p_con->original_addr + p_con->v_mem_limit - SCREEN_WIDTH) {
                    unsigned int goal_cursor =
                        p_con->original_addr +
                        SCREEN_WIDTH * ((p_con->cursor - p_con->original_addr) /
                                            SCREEN_WIDTH +
                                        1);
                    // p_con->cursor =
                    //     p_con->original_addr +
                    //     SCREEN_WIDTH *
                    //         ((p_con->cursor - p_con->original_addr) /
                    //         SCREEN_WIDTH +
                    //          1);
                    while (p_con->cursor < goal_cursor) {
                        *p_vmem++ = ' ';
                        *p_vmem++ = ENTER_COLOR;
                        p_con->cursor++;
                    }
                }
                // redo_undo
                //record.ch = ch;
                //record.delete_ch = 0;
                // push_redo_undo_stack(&undo_stack, record);
                // clear_redo_stack(&redo_stack);
                break;
            case '\b':  // 退格键
                if (p_con->cursor > p_con->original_addr) {
                    if (*(p_vmem - 1) == TAB_COLOR) {
                        // Lg: 处理TAB, 定位目标位置: 行开始+4*(行位置/4)、
                        while (p_con->cursor > p_con->original_addr) {
                            if (*(p_vmem - 1) == TAB_COLOR) {
                                *(p_vmem - 2) = ' ';
                                *(p_vmem - 1) = DEFAULT_CHAR_COLOR;
                                p_vmem -= 2;
                                p_con->cursor--;
                            } else
                                break;
                            if (((p_con->cursor - p_con->original_addr) %
                                 SCREEN_WIDTH) %
                                    TAB_SIZE ==
                                0)
                                break;
                        }
                        // redo_undo
                        //record.ch = ch;
                        //record.delete_ch = '\t';
                        // push_redo_undo_stack(&undo_stack, record);
                        // clear_redo_stack(&redo_stack);
                    } else if (*(p_vmem - 1) == ENTER_COLOR) {
                        // Lg: 处理换行，删除所有ENTER_COLOR, 直到上一行开始
                        unsigned int line_begin_cursor =
                            p_con->original_addr +
                            SCREEN_WIDTH *
                                ((p_con->cursor - p_con->original_addr) /
                                     SCREEN_WIDTH -
                                 1);
                        while (p_con->cursor > line_begin_cursor &&
                               *(p_vmem - 1) == ENTER_COLOR) {
                            *(p_vmem - 2) = ' ';
                            *(p_vmem - 1) = DEFAULT_CHAR_COLOR;
                            p_vmem -= 2;
                            p_con->cursor--;
                        }
                        // redo_undo
                        //record.ch = ch;
                        //record.delete_ch = '\n';
                        // push_redo_undo_stack(&undo_stack, record);
                        // clear_redo_stack(&redo_stack);
                    } else {
                        // 其他情况
                        p_con->cursor--;
                        char temp = *(p_vmem - 2);
                        *(p_vmem - 2) = ' ';
                        *(p_vmem - 1) = DEFAULT_CHAR_COLOR;
                        // redo_undo
                        //record.ch = ch;
                        //record.delete_ch = temp;
                        // push_redo_undo_stack(&undo_stack, record);
                        // clear_redo_stack(&redo_stack);
                    }
                }
                break;
            case '\t':  // Lg: 处理TAB, 定位目标位置: 行开始+4*(行位置/4+1)
                int tabCnt =
                    TAB_SIZE -
                    ((p_con->cursor - p_con->original_addr) % SCREEN_WIDTH) %
                        TAB_SIZE;
                for (int i = 0; i < tabCnt; i++) {
                    if (p_con->cursor <
                        p_con->original_addr + p_con->v_mem_limit - 1) {
                        *p_vmem++ = ' ';
                        *p_vmem++ = TAB_COLOR;
                        p_con->cursor++;
                    }
                }
                // redo_undo
                //record.ch = ch;
                //record.delete_ch = 0;
                // push_redo_undo_stack(&undo_stack, record);
                // clear_redo_stack(&redo_stack);
                break;
            case '\r':  // Lg: ESC
                mode = 1;
                init_search(p_con);
                return;
            default:
                if (p_con->cursor <
                    p_con->original_addr + p_con->v_mem_limit - 1) {
                    *p_vmem++ = ch;
                    *p_vmem++ = DEFAULT_CHAR_COLOR;
                    p_con->cursor++;

                    // redo_undo
                    //record.ch = ch;
                    //record.delete_ch = 0;
                    // push_redo_undo_stack(&undo_stack, record);
                    // clear_redo_stack(&redo_stack);
                }
                break;
        }
    } else if (mode == 1) {  // Lg: mode1 查找输入模式
        switch (ch) {
            case '\n':
                KMP(p_con);
                mode = 2;
                break;
            case '\b':  // 退格键
                if (p_con->cursor > p_con->search_cursor) {
                    p_con->cursor--;
                    char temp = *(p_vmem - 2);
                    *(p_vmem - 2) = ' ';
                    *(p_vmem - 1) = DEFAULT_CHAR_COLOR;
                    // redo_undo
                    //record.ch = ch;
                    //record.delete_ch = temp;
                    // push_redo_undo_stack(&search_undo_stack, record);
                    // clear_redo_stack(&search_redo_stack);
                }
                break;
                // case '\r':  // Lg: ESC
                //     mode = 1;
                //	init_search(p_con);
                //    return;
            case '\t':
                return;
            case '\r':
                return;
            default:
                if (p_con->cursor <
                    p_con->original_addr + p_con->v_mem_limit - 1) {
                    *p_vmem++ = ch;
                    *p_vmem++ = RED;
                    p_con->cursor++;
                    // redo_undo
                    //record.ch = ch;
                    //record.delete_ch = 0;
                    // push_redo_undo_stack(&search_undo_stack, record);
                    // clear_redo_stack(&search_redo_stack);
                }
                break;
        }
    } else if (mode == 2) {  // Lg: mode2 查找显示模式
        if (ch == '\r') {
            exit_search(p_con);
            mode = 0;
        }
    } else {
    }

    while (p_con->cursor >= p_con->current_start_addr + SCREEN_SIZE) {
        scroll_screen(p_con, SCR_DN);
    }

    flush(p_con);
}

/*======================================================================*
                           flush
*======================================================================*/
PRIVATE void flush(CONSOLE* p_con) {
    set_cursor(p_con->cursor);
    set_video_start_addr(p_con->current_start_addr);
}

/*======================================================================*
                            set_cursor
 *======================================================================*/
PRIVATE void set_cursor(unsigned int position) {
    disable_int();
    out_byte(CRTC_ADDR_REG, CURSOR_H);
    out_byte(CRTC_DATA_REG, (position >> 8) & 0xFF);
    out_byte(CRTC_ADDR_REG, CURSOR_L);
    out_byte(CRTC_DATA_REG, position & 0xFF);
    enable_int();
}

/*======================================================================*
                          set_video_start_addr
 *======================================================================*/
PRIVATE void set_video_start_addr(u32 addr) {
    disable_int();
    out_byte(CRTC_ADDR_REG, START_ADDR_H);
    out_byte(CRTC_DATA_REG, (addr >> 8) & 0xFF);
    out_byte(CRTC_ADDR_REG, START_ADDR_L);
    out_byte(CRTC_DATA_REG, addr & 0xFF);
    enable_int();
}

/*======================================================================*
                           select_console
 *======================================================================*/
PUBLIC void select_console(int nr_console) /* 0 ~ (NR_CONSOLES - 1) */
{
    if ((nr_console < 0) || (nr_console >= NR_CONSOLES)) {
        return;
    }

    nr_current_console = nr_console;

    set_cursor(console_table[nr_console].cursor);
    set_video_start_addr(console_table[nr_console].current_start_addr);
}

/*======================================================================*
                           scroll_screen
 *----------------------------------------------------------------------*
 滚屏.
 *----------------------------------------------------------------------*
 direction:
        SCR_UP	: 向上滚屏
        SCR_DN	: 向下滚屏
        其它	: 不做处理
 *======================================================================*/
PUBLIC void scroll_screen(CONSOLE* p_con, int direction) {
    if (direction == SCR_UP) {
        if (p_con->current_start_addr > p_con->original_addr) {
            p_con->current_start_addr -= SCREEN_WIDTH;
        }
    } else if (direction == SCR_DN) {
        if (p_con->current_start_addr + SCREEN_SIZE <
            p_con->original_addr + p_con->v_mem_limit) {
            p_con->current_start_addr += SCREEN_WIDTH;
        }
    } else {
    }

    set_video_start_addr(p_con->current_start_addr);
    set_cursor(p_con->cursor);
}

/*======================================================================*
                           Search
                init_searh exit_search KMP
*======================================================================*/
PUBLIC void init_search(CONSOLE* p_con) {
    p_con->search_cursor = p_con->cursor;
}

PUBLIC void exit_search(CONSOLE* p_con) {
    u8* p_vmem = (u8*)(V_MEM_BASE + p_con->original_addr * 2);
    for (int i = 0; i < p_con->search_cursor; i++) {
        if (*(p_vmem + 1) == RED) {
            *(p_vmem + 1) = DEFAULT_CHAR_COLOR;
        }
        p_vmem += 2;
    }
    for (int i = p_con->search_cursor; i < p_con->cursor; i++) {
        *p_vmem++ = ' ';
        *p_vmem++ = DEFAULT_CHAR_COLOR;
    }
    p_con->cursor = p_con->search_cursor;
    p_con->search_cursor = 0;
    // clear_redo_stack(&search_undo_stack);
    // clear_redo_stack(&search_redo_stack);
}

PUBLIC void KMP(CONSOLE* p_con) {
    if (p_con->cursor == p_con->search_cursor)
        return;  // 没有输入
    char str_text[p_con->search_cursor - p_con->original_addr + 1];
    unsigned int len_text = p_con->search_cursor - p_con->original_addr;
    char* str_pattern[p_con->cursor - p_con->search_cursor + 1];
    unsigned int len_pattern = p_con->cursor - p_con->search_cursor;

    u8* p_vmem = (u8*)(V_MEM_BASE + p_con->search_cursor * 2);
    for (int i = 0; i < len_pattern; i++) {
        str_pattern[i] = *p_vmem;
        p_vmem += 2;
    }

    p_vmem = (u8*)(V_MEM_BASE + p_con->original_addr * 2);
    for (int i = 0; i + len_pattern - 1 < len_text; i++) {
        int flag = 1;
        // 暴力匹配
        for (int j = 0; j < len_pattern; j++) {
            if (*(p_vmem + 2 * (i + j)) != str_pattern[j] ||
                *(p_vmem + 2 * (i + j) + 1) != DEFAULT_CHAR_COLOR) {
                flag = 0;
                break;
            }
        }
        if (!flag)
            continue;

        // 染色并指针跳转
        for (int j = 0; j < len_pattern; j++) {
            *(p_vmem + 2 * (i + j) + 1) = RED;
        }
        i += len_pattern - 1;
    }
}

///*======================================================================*
//                           redo_undo_methods
// *======================================================================*/
//PUBLIC int is_redo_undo_stack_empty(REDO_UNDO_STACK* stack) {
//    return stack->index == 0;
//}

//PUBLIC void push_redo_undo_stack(REDO_UNDO_STACK* stack, INPUT_RECORD record) {
//    if (stack->index == SCREEN_SIZE) {
//        for (int i = 0; i < SCREEN_SIZE - 1; i++) {
//            stack->input_record[i] = stack->input_record[i + 1];
//        }
//        stack->index = SCREEN_SIZE - 1;
//    }
//    stack->input_record[stack->index++] = record;
//}

//PUBLIC INPUT_RECORD pop_redo_undo_stack(REDO_UNDO_STACK* stack) {
//    if (stack->index == 0) {
//        INPUT_RECORD record;
//        record.ch = 0;
//        record.delete_ch = 0;
//        return record;
//    }
//    return stack->input_record[--stack->index];
//}

//PUBLIC void clear_redo_stack(REDO_UNDO_STACK* stack) {
//    stack->index = 0;
//}
///*======================================================================*
//                           undo
// *----------------------------------------------------------------------*
// 撤销
// *----------------------------------------------------------------------*
// 对撤销栈顶的操作压入重做栈，并进行逆向
// *======================================================================*/
//PUBLIC void undo(CONSOLE* p_con,
//                 REDO_UNDO_STACK* undo_stack,
//                 REDO_UNDO_STACK* redo_stack) {
//    if (is_redo_undo_stack_empty(undo_stack))
//        return;
//    INPUT_RECORD record = pop_redo_undo_stack(undo_stack);
//    push_redo_undo_stack(redo_stack, record);
//    if (record.ch == '\b') {
//        out_char(p_con, record.delete_ch);
//    } else {
//        out_char(p_con, '\b');
//    }
//}

///*======================================================================*
//                           redo
// *----------------------------------------------------------------------*
// 重做
// *----------------------------------------------------------------------*
// 对重做栈顶的操作压入撤销栈，并进行正向
// *======================================================================*/
//PUBLIC void redo(CONSOLE* p_con,
//                 REDO_UNDO_STACK* undo_stack,
//                 REDO_UNDO_STACK* redo_stack) {
//    if (is_redo_undo_stack_empty(redo_stack))
//        return;
//    INPUT_RECORD record = pop_redo_undo_stack(redo_stack);
//    push_redo_undo_stack(undo_stack, record);
//    out_char(p_con, record.ch);
//}