#define MAXINSTRUCTIONSIZE 50 
#define RIGHT 5
#define LEFT 4
#define UP 3
#define DOWN 2
#define BACKSPACE 127 

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

#define ASMPRELUDE "BITS 64\n\
\n\
global _start\n\
\n\
section .text\n\
\n\
_start:"

struct history {
    char instruction[MAXINSTRUCTIONSIZE];
    unsigned long long adr;
    int idx;
    struct history *next;
    struct history *prev;
};

struct history *add_to_history(struct history*);
struct history *find_head(struct history*);
struct history *find_by_adr(unsigned long long, struct history*);

