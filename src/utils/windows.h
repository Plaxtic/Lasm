#include <sys/user.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#define MAINWINWIDTH ((COLS/2) + 3)
#define MAINWINHEIGHT (LINES)
#define SUBWINWIDTH ((COLS/2) - 3)
#define SUBWINHEIGHT (LINES/2)

#define RIGHT 5
#define LEFT 4
#define CTL_C 3
#define UP 3 
#define DOWN 2
#define BACKSPACE 127 
#define CTL_BACKSPACE 263 

#include <ncurses.h>
#include <sys/wait.h>

WINDOW *create_newwin(int height, int width, int starty, int startx);
void clear_line(WINDOW *w);
void print_regs(WINDOW*, int, struct user_regs_struct*,  struct user_regs_struct*);
void print_flags(WINDOW*, int, struct user_regs_struct *, struct user_regs_struct*);
void print_stack(WINDOW *w, pid_t child, unsigned long long rsp, int rspoff, int stckln, int hdln, int lines);
struct history *get_instruction(WINDOW *w, struct history *curr, int x, int y);
