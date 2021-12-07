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
void print_regs(WINDOW *w, int pos, struct user_regs_struct *regs);
void print_flags(WINDOW *w, int pos, struct user_regs_struct *regs);
void print_stack(WINDOW *w, pid_t child, unsigned long long rsp, int rspoff, int stckln, int hdln, int lines);
struct history *get_instruction(WINDOW *w, struct history *curr, int x, int y);

WINDOW *create_newwin(int height, int width, int starty, int startx){
    WINDOW *local_win;

    local_win = newwin(height, width, starty, startx);
    box(local_win, '|' , '-');
    wrefresh(local_win);

    return local_win;
}

void clear_line(WINDOW *w) {
    wclrtoeol(w);
    box(w, '|' , '-');
    wrefresh(w);
}

struct history *get_instruction(WINDOW *w, struct history *curr, int x, int y) {
    int len = 0;
    int cursor = x;

    char ch;
    while ((ch = mvwgetch(w, y, cursor)) != '\n') {
        switch (ch) {

            case BACKSPACE:
                if (cursor > x) {

                    // delete last char
                    curr->instruction[--len] = 0;
                    mvwprintw(w, y, x, "%s", curr->instruction);

                    // decrement cursor
                    cursor--;
                    clear_line(w);
                }
                break;

            case RIGHT:
                // TODO
                break;

            case LEFT:
                // TODO
                break;

            case UP:
                if (curr->prev != NULL) {
                    curr = curr->prev;
                    mvwprintw(w, y, x, "%s", curr->instruction);
                    clear_line(w);

                    len = strlen(curr->instruction);
                    cursor = len + x;
                }
                break;

            case DOWN:
                if (curr->next != NULL) {
                    curr = curr->next;
                    mvwprintw(w, y, x, "%s", curr->instruction);
                    clear_line(w);

                    len = strlen(curr->instruction);
                    cursor = len + x;
                }
                break;

            default:
                if (cursor < MAINWINWIDTH-2 && cursor < MAXINSTRUCTIONSIZE+x) {
                    curr->instruction[len++] = ch;
                    mvwprintw(w, y, x, "%s", curr->instruction);
                    cursor++;
                }
                break;
        }
    }
    return curr;
}

char *hexdump(unsigned char *bytes, int len) {
    char *dump = malloc(len);

    for (int i = 0; i < len; ++i) {
        if (isprint(bytes[i]))
            dump[i] = bytes[i];
        else
            dump[i] = '.';
    }
    dump[len] = 0;

    return dump;
}

void print_stack(WINDOW *w, pid_t child, unsigned long long rsp, int rspoff, int stckln, int hdln, int lines) {
    char stack_buf[LONGSIZ+1];

    for (int i = 0; i < lines; i++) {

        // stack pointer
        mvwprintw(w, 3+i, rspoff, "0x%016llx> ", rsp+(i*16));

        // get 8 bytes of stack
        gettdata(child, rsp+((i*2)*LONGSIZ), stack_buf, LONGSIZ);
        stack_buf[LONGSIZ] = 0;

        // stack
        mvwprintw(w, 3+i, rspoff+stckln, "0x%016llx", *((long long *)stack_buf));

        // hexdump
        mvwprintw(w, 3+i, rspoff+(stckln*2)+hdln, "|%s", hexdump((uint8_t *)stack_buf, LONGSIZ));

        // get 8 more bytes of stack
        gettdata(child, rsp+((i*2+1)*LONGSIZ), stack_buf, LONGSIZ);
        stack_buf[LONGSIZ] = 0;

        // stack
        mvwprintw(w, 3+i, rspoff+(stckln*2), "0x%016llx", *((long long *)stack_buf));

        // hexdump
        mvwprintw(w, 3+i, rspoff+(stckln*2)+hdln+LONGSIZ+1, "%s|", hexdump((uint8_t *)stack_buf, LONGSIZ));
    }
}


void print_flags(WINDOW *w, int pos, struct user_regs_struct *regs){
    long long eflags = regs->eflags;

    mvwprintw(w, 2,  pos, "cs:      0x%016llx", regs->cs);
    mvwprintw(w, 3,  pos, "ss:      0x%016llx", regs->ss);
    mvwprintw(w, 4,  pos, "ds:      0x%016llx", regs->ds);
    mvwprintw(w, 5,  pos, "es:      0x%016llx", regs->es);
    mvwprintw(w, 6,  pos, "fs:      0x%016llx", regs->fs);
    mvwprintw(w, 7,  pos, "gs:      0x%016llx", regs->gs);
    mvwprintw(w, 8,  pos, "fs_base: 0x%016llx", regs->fs_base);
    mvwprintw(w, 9,  pos, "gs_base: 0x%016llx", regs->gs_base);
    mvwprintw(w, 10,  pos, "eflags:  0x%016llx", regs->eflags);
    mvwprintw(w, 12,  pos, "CF : %d", !!(eflags & CF));
    mvwprintw(w, 13,  pos, "PF : %d", !!(eflags & PF));
    mvwprintw(w, 14,  pos, "AF : %d", !!(eflags & AF));
    mvwprintw(w, 15,  pos, "ZF : %d", !!(eflags & ZF));
    mvwprintw(w, 16,  pos, "SF : %d", !!(eflags & SF));
    mvwprintw(w, 16,  pos, "TF : %d", !!(eflags & TF));
    mvwprintw(w, 17,  pos, "IF : %d", !!(eflags & IF));
    mvwprintw(w, 18,  pos, "DF : %d", !!(eflags & DF));
    mvwprintw(w, 12,  pos+9, "OF   : %d", !!(eflags & OF));
    mvwprintw(w, 13,  pos+9, "IOPL : %d", !!(eflags & IOPL));
    mvwprintw(w, 14,  pos+9, "NT   : %d", !!(eflags & NT));
    mvwprintw(w, 15,  pos+9, "RF   : %d", !!(eflags & RF));
    mvwprintw(w, 16,  pos+9, "VM   : %d", !!(eflags & VM));
    mvwprintw(w, 17,  pos+9, "AC   : %d", !!(eflags & AC));
    mvwprintw(w, 18,  pos+9, "VIF  : %d", !!(eflags & VIF));
    mvwprintw(w, 12,  pos+19, "VIP : %d", !!(eflags & VIP));
    mvwprintw(w, 13,  pos+19, "ID  : %d", !!(eflags & ID));
}


void print_regs(WINDOW *w, int pos, struct user_regs_struct *regs) {
    mvwprintw(w, 2,  pos, "rax: 0x%016llx", regs->rax);
    mvwprintw(w, 3,  pos, "rbx: 0x%016llx", regs->rbx);
    mvwprintw(w, 4,  pos, "rcx: 0x%016llx", regs->rcx);
    mvwprintw(w, 5,  pos, "rdx: 0x%016llx", regs->rdx);
    mvwprintw(w, 6,  pos, "rdi: 0x%016llx", regs->rdi);
    mvwprintw(w, 7,  pos, "rsi: 0x%016llx", regs->rsi);
    mvwprintw(w, 8,  pos, "rip: 0x%016llx", regs->rip);
    mvwprintw(w, 9,  pos, "rsp: 0x%016llx", regs->rsp);
    mvwprintw(w, 10, pos, "rbp: 0x%016llx", regs->rbp);
    mvwprintw(w, 11, pos, "r8:  0x%016llx", regs->r8);
    mvwprintw(w, 12, pos, "r9:  0x%016llx", regs->r9);
    mvwprintw(w, 13, pos, "r10: 0x%016llx", regs->r10);
    mvwprintw(w, 14, pos, "r11: 0x%016llx", regs->r11);
    mvwprintw(w, 15, pos, "r12: 0x%016llx", regs->r12);
    mvwprintw(w, 16, pos, "r13: 0x%016llx", regs->r13);
    mvwprintw(w, 17, pos, "r14: 0x%016llx", regs->r14);
    mvwprintw(w, 18, pos, "r15: 0x%016llx", regs->r15);
}


