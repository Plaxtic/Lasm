#include <keystone/keystone.h>
#include <sys/ptrace.h>
#include <sys/wait.h>
#include <sys/user.h>
#include <sys/reg.h>
#include <unistd.h>
#include <string.h>
#include <ncurses.h>

#include "utils/syscalls.h"
#include "utils/history.h"
#include "utils/windows.h"
#include "utils/labels.h"

bool is_jump(char*op);
char *get_last_word(char *op);
pid_t run_trace(char*);
uint8_t *assemble(const char *, size_t*, ks_engine*);

int main(int argc, char *argv[]) {

    // get options
    bool is_output = false;
    FILE *outfd;
    if (argc > 1) {
        int op;

        while ((op = getopt(argc, argv, "o:")) != -1) {
            switch (op) {

                // output file
                case 'o':
                    outfd = fopen(optarg, "w");

                    if (outfd == NULL) {
                        perror("output");
                        return 1;
                    }

                    is_output = true;
                    fputs(ASMPRELUDE, outfd);
                    break;

                default: 
                    fprintf(stderr, "unrecognized option '%c'", op);
                    return 1;
            }
        }
    }

    // init ncurses
    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);

    // set dimensions 
    int height = LINES/2;
    int width = (COLS/2) - 3;
    int starty = (LINES - height);
    int startx = (COLS - width);

    // create 3 windows
    WINDOW *registers = create_newwin(height, width, starty, startx);
    WINDOW *stack = create_newwin(height, width, 0, startx);
    WINDOW *instructions = create_newwin(LINES, MAINWINWIDTH, 0, 0);
    keypad(instructions, TRUE);

    // run program in child process and trace with parent
    pid_t child = run_trace("src/x86/nul");

    // initialize assembler
    ks_engine *ks;
    if (ks_open(KS_ARCH_X86, KS_MODE_64, &ks) != KS_ERR_OK) 
        return -1;

    // get status
    int status;
    wait(&status);

    // load blank instruction
    struct history *tmp, *curr;
    tmp = curr = malloc(sizeof(struct history));
    memset(curr->instruction, 0, MAXINSTRUCTIONSIZE);
    curr->next = curr->prev  = NULL;

    // initalise label list
    struct label *tmpl, *labels;
    tmpl = labels = NULL;

    struct user_regs_struct regs;
    int addr_oset = 28;
    int y = 2;
    while (1) {

        // clear and jump to top if at bottom
        if (y > LINES-3) {
            y = 2;
            wclear(instructions);
            box(instructions, '|' , '-');
            wrefresh(instructions);
        }

        // get register state 
        ptrace(PTRACE_GETREGS, child, 0, &regs);

        // stack headings
        mvwprintw(stack, 2, width/7, "rsp");
        mvwprintw(stack, 1, width/2-3, "STACK");
        mvwprintw(stack, 2, (width)-width/5, "hexdump");

        // print stack, pointer, and hex dump
        print_stack(stack, child, regs.rsp, 2, 20, 19, 15);
        wrefresh(stack);

        // update registers
        mvwprintw(registers, 1, 11, "REGISTERS");
        print_regs(registers, 4, &regs);
        wrefresh(registers);

        // update flags 
        mvwprintw(registers, 1, width-20, "FLAGS");
        print_flags(registers, width-31, &regs);
        wrefresh(registers);

        // print current address (rip) 
        mvwprintw(instructions, y, addr_oset, "[%#08llx]> ", regs.rip);
        wrefresh(instructions);

        // get instruction
        curr = get_instruction(instructions, curr, 40, y);

        // skip enter or deleted line
        if (!curr->instruction[0]) continue;

        // if q, end loop
        else if (strcmp(curr->instruction, "q") == 0) break;

        // step special command
        else if (!strcmp(curr->instruction, "s") || 
                 !strncmp(curr->instruction, "s ", 2)) {
            clear_line(instructions);
            int nsteps, i = 0;

            // skip whitespace
            while (curr->instruction[++i] == ' ');

            // mininum one step
            if (!(nsteps = strtoul(&curr->instruction[i], NULL, 10))) nsteps = 1;

            // take nstep's
            for (i = 0; i < nsteps; ++i) {
                if (ptrace(PTRACE_SINGLESTEP, child, NULL, NULL) < 0) {
                    perror("Step Fail");
                    return 1;
                }
                wait(&status);
            }
            wmove(instructions, y, 40);
            clear_line(instructions);
        }

        else {
            // if last operand is a label, replace with address
            // ---------- ONLY WORKING FOR JUMPS -------------
            // ------- (maybe only should) ---------
            if (is_jump(curr->instruction)) {
                long long adr;
                char *lastword = get_last_word(curr->instruction);

                adr = get_adr_by_name(labels, lastword);
                if (adr > 0)
                    snprintf(lastword, MAXINSTRUCTIONSIZE, "%lld", adr-regs.rip);
            }

            // try assemble buffer
            size_t nbytes;
            uint8_t *bytes = assemble(curr->instruction, &nbytes, ks);
            if (nbytes > 0) {

                // cover "Invalid" with spaces
                mvwprintw(instructions, y, 2, "           ");

                // print bytecode in hex
                for (int i = 0; i < nbytes; i++)
                    mvwprintw(instructions, y, 2+(i*2), "%02x", bytes[i]);
                y++;

                // inject raw assembled bytes to instruction pointer (rip)
                puttdata(child, regs.rip, bytes, nbytes);

                // set process to break on step
                if (ptrace(PTRACE_SINGLESTEP, child, NULL, NULL) < 0) {
                    perror("Step Fail");
                    return 1;
                }

                // try execute bytes
                wait(&status);

                // save to file
                if (is_output) 
                    fprintf(outfd, "\t%s\n", curr->instruction);
            }
            else {

                // set label at address
                if (is_label(curr->instruction)) {
                    wmove(instructions, y, 2);
                    clear_line(instructions);

                    tmpl = addlabel(labels, curr->instruction, regs.rip);
                    if (tmpl == NULL) {
                        mvwprintw(instructions, y, 2, "Bad Label!");
                    }
                    else {
                        labels = tmpl;
                        mvwprintw(instructions, y, 2, "%s:", labels->name);
                        y++;
                    }
                }
                else {

                    // failed to assemble
                    mvwprintw(instructions, y, 2, "Invalid" );
                    clear_line(instructions);
                }
            }
        }

        if (curr->next != NULL) {

            // if you are in the middle of history, save command and preserve chain 
            tmp = curr;
            curr = find_head(curr);
            memset(curr->instruction, 0, MAXINSTRUCTIONSIZE);
            strncpy(curr->instruction, tmp->instruction, MAXINSTRUCTIONSIZE);
        }
        curr = add_to_history(curr);
    }
    return 0;
}

pid_t run_trace(char *filename) {
    pid_t child = fork();

    if (!child) {
        ptrace(PTRACE_TRACEME, 0, NULL, NULL);
        execl(filename, filename, NULL);
        exit(0);
    }
    return child;
}

uint8_t *assemble(const char *code, size_t *nbytes, ks_engine *ks ) {
    uint8_t *encode;
    size_t count;

    // compile CODE from 0 into "encode" of "size" bytes 
    if (ks_asm(ks, code, 0, &encode, nbytes, &count) != KS_ERR_OK)
        return NULL;

    return encode;
}

char *get_last_word(char *op) {
    int len = strlen(op);
    char *p = op;
    
    p += len;

    while (*--p != ' ' && op < p);

    return p > op ? p+1 : op;
}
 
bool is_jump(char*op) {
    // TODO
    return op[0] == 'j' || op[0] == 'J';
}
