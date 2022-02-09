#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define MAXINSTRUCTIONSIZE 48

#define ASMPRELUDE "BITS 64\n\
\n\
global _start\n\
\n\
section .text\n\
\n\
_start:"

struct history {
    char instruction[MAXINSTRUCTIONSIZE];
    unsigned long long addr;
    struct history *next;
    struct history *prev;
};

struct history *add_to_history(struct history*);
struct history *find_head(struct history*);
struct history *find_by_adr(unsigned long long, struct history*);
struct history *load_history(FILE*);

