#include "ipc.h"
#include "windows.h"
#include "history.h"

void init_ncurses() {
    initscr();
    raw();
    noecho();
    keypad(stdscr, TRUE);
    if (LINES < 40 || COLS < 170) {
        fprintf(stderr, "Terminal is too small, continue? [N/y] ");

        if (toupper(getchar()) != 'Y') {
            endwin();
            exit(EXIT_SUCCESS);
        }
    }
}

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
    int len = strlen(curr->instruction);  // Start with existing string length
    int cursor_pos = len;  // Start cursor at end
    int screen_cursor = x + len;  // Screen position

    int ch;
    while ((ch = mvwgetch(w, y, screen_cursor)) != '\n') {
        switch (ch) {

            case BACKSPACE:
                if (cursor_pos > 0) {
                    //
                    // Shift everything left from cursor position
                    memmove(&curr->instruction[cursor_pos - 1], &curr->instruction[cursor_pos], len - cursor_pos + 1);

                    len--;
                    cursor_pos--;
                    screen_cursor--;

                    curr->instruction[len] = '\0';

                    // redraw
                    wmove(w, y, x);
                    wclrtoeol(w);
                    wprintw(w, "%s", curr->instruction);
                    wmove(w, y, screen_cursor);
                    box(w, '|', '-');
                    wrefresh(w);
                }
                break;

            case CTL_BACKSPACE:
                // Word delete - delete from cursor back to previous space/start
                if (cursor_pos > 0) {
                    int start_pos = cursor_pos;

                    // Skip current spaces
                    while (cursor_pos > 0 && curr->instruction[cursor_pos - 1] == ' ') {
                        cursor_pos--;
                    }
                    // Delete word
                    while (cursor_pos > 0 && curr->instruction[cursor_pos - 1] != ' ') {
                        cursor_pos--;
                    }

                    // Shift remaining text left
                    memmove(&curr->instruction[cursor_pos],
                            &curr->instruction[start_pos],
                            len - start_pos + 1);

                    len -= (start_pos - cursor_pos);
                    screen_cursor = x + cursor_pos;

                    // Redraw
                    mvwprintw(w, y, x, "%s ", curr->instruction);
                    wmove(w, y, screen_cursor);
                    clear_line(w);
                }
                break;

            case KEY_RIGHT:
                if (cursor_pos < len) {
                    cursor_pos++;
                    screen_cursor++;
                }
                break;

            case KEY_LEFT:
                if (cursor_pos > 0) {
                    cursor_pos--;
                    screen_cursor--;
                }
                break;

            case CTL_C:
                return NULL;

            case KEY_UP:
                if (curr->prev != NULL) {
                    curr = curr->prev;
                    len = strlen(curr->instruction);
                    cursor_pos = len;  // Put cursor at end
                    screen_cursor = x + len;
                    mvwprintw(w, y, x, "%s", curr->instruction);
                    clear_line(w);
                }
                break;

            case KEY_DOWN:
                if (curr->next != NULL) {
                    curr = curr->next;
                    len = strlen(curr->instruction);
                    cursor_pos = len;  // Put cursor at end
                    screen_cursor = x + len;
                    mvwprintw(w, y, x, "%s", curr->instruction);
                    clear_line(w);
                }
                break;

            default:
                if (screen_cursor < MAINWINWIDTH-2 &&
                    len < MAXINSTRUCTIONSIZE-1 &&
                    isprint(ch)) {

                    // Insert character at cursor position
                    memmove(&curr->instruction[cursor_pos + 1],
                            &curr->instruction[cursor_pos],
                            len - cursor_pos + 1);  // +1 for null terminator

                    curr->instruction[cursor_pos] = ch;
                    len++;
                    cursor_pos++;
                    screen_cursor++;

                    // Redraw from current position
                    mvwprintw(w, y, x, "%s", curr->instruction);
                    wmove(w, y, screen_cursor);
                }
                break;
        }
    }
    return curr;
}

//struct history *get_instruction(WINDOW *w, struct history *curr, int x, int y) {
//    int len = 0;
//    int cursor = x;
//
//    int ch;
//    while ((ch = mvwgetch(w, y, cursor)) != '\n') {
//        switch (ch) {
//            case BACKSPACE:
//                if (cursor > x) {
//
//                    // delete last char
//                    curr->instruction[--len] = 0;
//                    mvwprintw(w, y, x, "%s", curr->instruction);
//
//                    // decrement cursor
//                    cursor--;
//                    clear_line(w);
//                }
//                break;
//
//            case CTL_BACKSPACE:
//                if (curr->instruction[len-1] == ' ') {
//                    while (cursor > x && curr->instruction[len-1] == ' ') {
//                        curr->instruction[len--] = 0;
//                        cursor--;
//                        mvwprintw(w, y, x, "%s", curr->instruction);
//                        wmove(w, y, cursor);
//                        clear_line(w);
//                    }
//                }
//                else {
//                    while (cursor > x && curr->instruction[len-1] != ' ') {
//                        curr->instruction[len--] = 0;
//                        cursor--;
//                        mvwprintw(w, y, x, "%s", curr->instruction);
//                        wmove(w, y, cursor);
//                        clear_line(w);
//                    }
//                }
//                break;
//
//            case KEY_RIGHT:
//                // TODO
//                break;
//
//            case CTL_C:
//                return NULL;
//
//            case KEY_LEFT:
//                // TODO
//                break;
//
//            case KEY_UP:
//                if (curr->prev != NULL) {
//
//                    // move back in history
//                    curr = curr->prev;
//                    mvwprintw(w, y, x, "%s", curr->instruction);
//                    clear_line(w);
//
//                    // reset len and cursor
//                    len = strlen(curr->instruction);
//                    cursor = len + x;
//                }
//                break;
//
//            case KEY_DOWN:
//                if (curr->next != NULL) {
//
//                    // move forward in history
//                    curr = curr->next;
//                    mvwprintw(w, y, x, "%s", curr->instruction);
//                    clear_line(w);
//
//                    // reset len and cursor
//                    len = strlen(curr->instruction);
//                    cursor = len + x;
//                }
//                break;
//
//            default:
//                if (cursor < MAINWINWIDTH-2 && 
//                    cursor < MAXINSTRUCTIONSIZE+x &&
//                    isprint(ch)) {
//
//                    // save char
//                    curr->instruction[len++] = ch;
//
//                    // echo
//                    mvwprintw(w, y, x, "%s", curr->instruction);
//                    cursor++;
//                }
//                break;
//        }
//    }
//    return curr;
//}

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
    char *hexdump_str;

    for (int i = 0; i < lines; i++) {

        // stack pointer
        mvwprintw(w, 3+i, rspoff, "0x%016llx> ", rsp+(i*16));

        // get 8 bytes of stack
        gettdata(child, rsp+((i*2)*LONGSIZ), stack_buf, LONGSIZ);
        stack_buf[LONGSIZ] = 0;

        // stack
        mvwprintw(w, 3+i, rspoff+stckln, "0x%016llx", *((long long *)stack_buf));

        // hexdump
        hexdump_str = hexdump((uint8_t *)stack_buf, LONGSIZ);
        mvwprintw(w, 3+i, rspoff+(stckln*2)+hdln, "|%s", hexdump_str);
        free(hexdump_str);

        // get 8 more bytes of stack
        gettdata(child, rsp+((i*2+1)*LONGSIZ), stack_buf, LONGSIZ);
        stack_buf[LONGSIZ] = 0;

        // stack
        mvwprintw(w, 3+i, rspoff+(stckln*2), "0x%016llx", *((long long *)stack_buf));

        // hexdump
        hexdump_str = hexdump((uint8_t *)stack_buf, LONGSIZ);
        mvwprintw(w, 3+i, rspoff+(stckln*2)+hdln+LONGSIZ+1, "%s|", hexdump_str);
        free(hexdump_str);
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
    mvwprintw(w, 7,  pos, "%c rsi: 0x%016llx", regsb->rsi == regsa->rsi ? ' ' : '*', regsa->rsi);
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
