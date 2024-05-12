
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			      console.h
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
						    Forrest Yu, 2005
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

#ifndef _ORANGES_CONSOLE_H_
#define _ORANGES_CONSOLE_H_

/* CONSOLE */
typedef struct s_console
{
	unsigned int	current_start_addr;	/* 当前显示到了什么位置	  */
	unsigned int	original_addr;		/* 当前控制台对应显存位置 */
	unsigned int	v_mem_limit;		/* 当前控制台占的显存大小 */
	unsigned int	cursor;			/* 当前光标位置 */
	unsigned int	search_cursor;		/*进入查找时的光标位置*/
}CONSOLE;


#define SCR_UP	1	/* scroll forward */
#define SCR_DN	-1	/* scroll backward */

#define SCREEN_SIZE		(80 * 25)
#define SCREEN_WIDTH		80

#define	TAB_SIZE	4
#define	COLOR_TAB	0xFF

#define DEFAULT_CHAR_COLOR	0x07	/* 0000 0111 黑底白字 */

typedef struct input_record {
	char ch;
	char delete_ch;
}INPUT_RECORD;

typedef struct redo_undo_stack {
	INPUT_RECORD input_record[SCREEN_SIZE];
	int index;
}REDO_UNDO_STACK;

#endif /* _ORANGES_CONSOLE_H_ */
