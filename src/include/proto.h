
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                            proto.h
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                                                    Forrest Yu, 2005
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

/* klib.asm */
PUBLIC void	out_byte(u16 port, u8 value);
PUBLIC u8	in_byte(u16 port);
PUBLIC void	disp_str(char * info);
PUBLIC void	disp_color_str(char * info, int color);
PUBLIC void disable_int();	// Lg: 关中断
PUBLIC void enable_int();	// Lg: 开中断

/* protect.c */
PUBLIC void	init_prot();
PUBLIC u32	seg2phys(u16 seg);

/* klib.c */
PUBLIC void	delay(int time);

/* kernel.asm */
void restart();

/* main.c */
void Reporter();
void R1();
void R2();
void R3();
void W1();
void W2();

/* i8259.c */
PUBLIC void put_irq_handler(int irq, irq_handler handler);
PUBLIC void spurious_irq(int irq);

/* clock.c */
PUBLIC void clock_handler(int irq);
PUBLIC void init_clock();

/* keyboard.c */
PUBLIC void init_keyboard();

/* tty.c */
PUBLIC void task_tty();
PUBLIC void in_process(TTY* p_tty, u32 key);

/* console.c */
PUBLIC void out_char(CONSOLE* p_con, char ch);
PUBLIC void scroll_screen(CONSOLE* p_con, int direction);

/* 以下是系统调用相关 */

// Lg: 系统级
/* proc.c */
PUBLIC  int     sys_get_ticks();        /* sys_call */
PUBLIC	void 	sys_sleep_ms(int milli_sec);
PUBLIC	void 	sys_print_str(char* buf, int len);
PUBLIC	void 	p_process(SEMAPHORE* sem);
PUBLIC	void 	v_process(SEMAPHORE* sem);

// Lg: 用户级
/* syscall.asm */
PUBLIC  void    sys_call();             /* int_handler */
PUBLIC  int     get_ticks();
PUBLIC	void 	sleep_ms(int milli_sec);
PUBLIC	void 	print_str(char* buf, int color);
PUBLIC	void 	P(SEMAPHORE* sem);
PUBLIC	void 	V(SEMAPHORE* sem);

