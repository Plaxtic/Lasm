#ifndef WINDOWS_H
#define WINDOWS_H

#include <sys/user.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <ncurses.h>

#include "syscalls.h"
#include "windows.h"
#include "history.h"

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

    int ch;
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

            case CTL_BACKSPACE:
                if (curr->instruction[len-1] == ' ') {
                    while (cursor > x && curr->instruction[len-1] == ' ') {
                        curr->instruction[len--] = 0;
                        cursor--;
                        mvwprintw(w, y, x, "%s", curr->instruction);
                        wmove(w, y, cursor);
                        clear_line(w);

                    }
                }
                else {
                    while (cursor > x && curr->instruction[len-1] != ' ') {
                        curr->instruction[len--] = 0;
                        cursor--;
                        mvwprintw(w, y, x, "%s", curr->instruction);
                        wmove(w, y, cursor);
                        clear_line(w);
                    }
                }
                break;

            case KEY_RIGHT:
                // TODO
                break;

            case CTL_C:
                return NULL;

            case KEY_LEFT:
                // TODO
                break;

            case KEY_UP:
                if (curr->prev != NULL) {

                    // move back in history
                    curr = curr->prev;
                    mvwprintw(w, y, x, "%s", curr->instruction);
                    clear_line(w);

                    // reset len and cursor
                    len = strlen(curr->instruction);
                    cursor = len + x;
                }
                break;

            case KEY_DOWN:
                if (curr->next != NULL) {

                    // move forward in history
                    curr = curr->next;
                    mvwprintw(w, y, x, "%s", curr->instruction);
                    clear_line(w);

                    // reset len and cursor
                    len = strlen(curr->instruction);
                    cursor = len + x;
                }
                break;

            default:
                if (cursor < MAINWINWIDTH-2 && cursor < MAXINSTRUCTIONSIZE+x) {

                    // save char
                    curr->instruction[len++] = ch;

                    // echo
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


void print_flags(WINDOW *w, int pos, struct user_regs_struct *regsb, 
                                     struct user_regs_struct *regsa){
    long long eflagsa = regsa->eflags;
    long long eflagsb = regsb->eflags;

    mvwprintw(w, 2,  pos, "%c cs:      0x%016llx", regsa->cs == regsb->cs ? ' ' : '*', regsa->cs);
    mvwprintw(w, 3,  pos, "%c ss:      0x%016llx", regsa->ss == regsb->ss ? ' ' : '*', regsa->ss);
    mvwprintw(w, 4,  pos, "%c ds:      0x%016llx", regsa->ds == regsb->ds ? ' ' : '*', regsa->ds);
    mvwprintw(w, 5,  pos, "%c es:      0x%016llx", regsa->es == regsb->es ? ' ' : '*', regsa->es);
    mvwprintw(w, 6,  pos, "%c fs:      0x%016llx", regsa->fs == regsb->fs ? ' ' : '*', regsa->fs);
    mvwprintw(w, 7,  pos, "%c gs:      0x%016llx", regsa->gs == regsb->gs ? ' ' : '*', regsa->gs);
    mvwprintw(w, 8,  pos, "%c fs_base: 0x%016llx", regsa->fs_base == regsb->fs_base ? ' ' : '*', regsa->fs_base);
    mvwprintw(w, 9,  pos, "%c gs_base: 0x%016llx", regsa->gs_base == regsb->gs_base ? ' ' : '*', regsa->gs_base);
    mvwprintw(w, 10,  pos,"%c eflags:  0x%016llx", eflagsa == eflagsb ? ' ' : '*', eflagsa);
    mvwprintw(w, 12,  pos,   "%c CF : %d", (eflagsa & CF) == (eflagsb & CF) ? ' ' : '*', !!(eflagsa & CF));
    mvwprintw(w, 13,  pos,   "%c PF : %d", (eflagsa & PF) == (eflagsb & PF) ? ' ' : '*', !!(eflagsa & PF));
    mvwprintw(w, 14,  pos,   "%c AF : %d", (eflagsa & AF) == (eflagsb & AF) ? ' ' : '*', !!(eflagsa & AF));
    mvwprintw(w, 15,  pos,   "%c ZF : %d", (eflagsa & ZF) == (eflagsb & ZF) ? ' ' : '*', !!(eflagsa & ZF));
    mvwprintw(w, 16,  pos,   "%c SF : %d", (eflagsa & SF) == (eflagsb & SF) ? ' ' : '*', !!(eflagsa & SF));
    mvwprintw(w, 16,  pos,   "%c TF : %d", (eflagsa & TF) == (eflagsb & TF) ? ' ' : '*', !!(eflagsa & TF));
    mvwprintw(w, 17,  pos,   "%c IF : %d", (eflagsa & IF) == (eflagsb & IF) ? ' ' : '*', !!(eflagsa & IF));
    mvwprintw(w, 18,  pos,   "%c DF : %d", (eflagsa & DF) == (eflagsb & DF) ? ' ' : '*', !!(eflagsa & DF));
    mvwprintw(w, 12,  pos+9, "%c OF   : %d", (eflagsa & OF) == (eflagsb & OF) ? ' ' : '*', !!(eflagsa & OF));
    mvwprintw(w, 13,  pos+9, "%c IOPL : %d", (eflagsa & IOPL) == (eflagsb & IOPL) ? ' ' : '*', !!(eflagsa & IOPL));
    mvwprintw(w, 14,  pos+9, "%c NT   : %d", (eflagsa & NT) == (eflagsb & NT) ? ' ' : '*', !!(eflagsa & NT));
    mvwprintw(w, 15,  pos+9, "%c RF   : %d", (eflagsa & RF) == (eflagsb & RF) ? ' ' : '*', !!(eflagsa & RF));
    mvwprintw(w, 16,  pos+9, "%c VM   : %d", (eflagsa & VM) == (eflagsb & VM) ? ' ' : '*', !!(eflagsa & VM));
    mvwprintw(w, 17,  pos+9, "%c AC   : %d", (eflagsa & AC) == (eflagsb & AC) ? ' ' : '*', !!(eflagsa & AC));
    mvwprintw(w, 18,  pos+9, "%c VIF  : %d", (eflagsa & VIF) == (eflagsb & VIF) ? ' ' : '*', !!(eflagsa & VIF));
    mvwprintw(w, 12,  pos+19,"%c VIP : %d", (eflagsa & VIP) == (eflagsb & VIP) ? ' ' : '*', !!(eflagsa & VIP));
    mvwprintw(w, 13,  pos+19,"%c ID  : %d", (eflagsa &  ID) == (eflagsb &  ID) ? ' ' : '*', !!(eflagsa & ID) );
}


void print_regs(WINDOW *w, int pos, struct user_regs_struct *regsb, 
                                    struct user_regs_struct *regsa) {
    mvwprintw(w, 2,  pos, "%c rax: 0x%016llx", regsb->rax == regsa->rax ? ' ' : '*', regsa->rax);
    mvwprintw(w, 3,  pos, "%c rbx: 0x%016llx", regsb->rbx == regsa->rbx ? ' ' : '*', regsa->rbx);
    mvwprintw(w, 4,  pos, "%c rcx: 0x%016llx", regsb->rcx == regsa->rcx ? ' ' : '*', regsa->rcx);
    mvwprintw(w, 5,  pos, "%c rdx: 0x%016llx", regsb->rdx == regsa->rdx ? ' ' : '*', regsa->rdx);
    mvwprintw(w, 6,  pos, "%c rdi: 0x%016llx", regsb->rdi == regsa->rdi ? ' ' : '*', regsa->rdi);
    mvwprintw(w, 7,  pos, "%c rsi: 0x%016llx", regsb->rsi == regsa->rdi ? ' ' : '*', regsa->rsi);
    mvwprintw(w, 8,  pos, "%c rip: 0x%016llx", regsb->rip == regsa->rip ? ' ' : '*', regsa->rip);
    mvwprintw(w, 9,  pos, "%c rsp: 0x%016llx", regsb->rsp == regsa->rsp ? ' ' : '*', regsa->rsp);
    mvwprintw(w, 10, pos, "%c rbp: 0x%016llx", regsb->rbp == regsa->rbp ? ' ' : '*', regsa->rbp);
    mvwprintw(w, 11, pos, "%c r8:  0x%016llx", regsb->r8 == regsa->r8   ? ' ' : '*', regsa->r8 );
    mvwprintw(w, 12, pos, "%c r9:  0x%016llx", regsb->r9 == regsa->r9   ? ' ' : '*', regsa->r9 );
    mvwprintw(w, 13, pos, "%c r10: 0x%016llx", regsb->r10 == regsa->r10 ? ' ' : '*', regsa->r10);
    mvwprintw(w, 14, pos, "%c r11: 0x%016llx", regsb->r11 == regsa->r11 ? ' ' : '*', regsa->r11);
    mvwprintw(w, 15, pos, "%c r12: 0x%016llx", regsb->r12 == regsa->r12 ? ' ' : '*', regsa->r12);
    mvwprintw(w, 16, pos, "%c r13: 0x%016llx", regsb->r13 == regsa->r13 ? ' ' : '*', regsa->r13);
    mvwprintw(w, 17, pos, "%c r14: 0x%016llx", regsb->r14 == regsa->r14 ? ' ' : '*', regsa->r14);
    mvwprintw(w, 18, pos, "%c r15: 0x%016llx", regsb->r15 == regsa->r15 ? ' ' : '*', regsa->r15);
}
#endif
