#ifndef WINDOWS_H
#define WINDOWS_H

#include <sys/user.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <ncurses.h>
#include <sys/wait.h>

// eflags
#define CF 0x0001 	
#define PF 0x0004 	
#define AF 0x0010 	
#define ZF 0x0040 	
#define SF 0x0080 	
#define TF 0x0100 	
#define IF 0x0200 	
#define DF 0x0400 	
#define OF 0x0800 	
#define IOPL 0x3000 	
#define NT  0x4000 	
#define RF  0x00010000 	
#define VM  0x00020000 	
#define AC  0x00040000 	
#define VIF 0x00080000 	
#define VIP 0x00100000 	
#define ID  0x00200000 	

// key numbers
#define RIGHT 5
#define LEFT 4
#define CTL_C 3
#define UP 3 
#define DOWN 2
#define BACKSPACE 263 // 127 
#define CTL_BACKSPACE 8 // 263 

// dimensions
#define MAINWINWIDTH ((COLS/2) + 3)
#define MAINWINHEIGHT (LINES)
#define SUBWINWIDTH ((COLS/2) - 3)
#define SUBWINHEIGHT (LINES/2)

void init_ncurses();
WINDOW *create_newwin(int, int, int, int);
struct history *get_instruction(WINDOW*, struct history*, int, int);
void clear_line(WINDOW*);
void print_stack(WINDOW *, pid_t, unsigned long long, int, int, int, int);
void print_regs(WINDOW*, int, struct user_regs_struct*,  
        struct user_regs_struct*);
void print_flags(WINDOW*, int, struct user_regs_struct *, 
        struct user_regs_struct*);
void update_ui(WINDOW *stack, WINDOW *registers, WINDOW *instructions, 
        struct user_regs_struct *regsb, struct user_regs_struct *regsa, 
        pid_t child, int oset, int y);

#endif
