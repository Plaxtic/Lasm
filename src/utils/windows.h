#define MAINWINWIDTH (COLS/2) + 3 

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

WINDOW *create_newwin(int height, int width, int starty, int startx);
void clear_line(WINDOW *w);
void print_regs(WINDOW*, int, struct user_regs_struct*,  struct user_regs_struct*);
void print_flags(WINDOW*, int, struct user_regs_struct *, struct user_regs_struct*);
void print_stack(WINDOW *w, pid_t child, unsigned long long rsp, int rspoff, int stckln, int hdln, int lines);
struct history *get_instruction(WINDOW *w, struct history *curr, int x, int y);
