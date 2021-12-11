#include <keystone/keystone.h>
#include <sys/ptrace.h>
#include <sys/wait.h>
#include <sys/user.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <ncurses.h>

#define ARCH KS_ARCH_X86

#include "utils/syscalls.h"
#include "utils/history.h"
#include "utils/windows.h"
#include "utils/labels.h"
#include "utils/syntax.h"


#define USAGE "Usage: %s\n\
\n\
\t-o <outfile>\n\
\t-s <asm syntax>\n\
\t-b <bits>\n\n"


void print_usage(char*);
pid_t run_trace(char*);
uint8_t *assemble(const char *, size_t*, ks_engine*);
int get_regs(pid_t child, struct user_regs_struct *regs);

int main(int argc, char *argv[]) {
    char filename[6] = "nul";

    // get options
    int bits = KS_MODE_64;
    int syntax = -1;
    bool no_prelude = false;
    FILE *outfd = NULL;
    int op;
    while ((op = getopt(argc, argv, "b:s:o:hp")) != -1) {
        switch (op) {

            // suppress prelude
            case 'p': 
                no_prelude = true;
                break;

            // output file
            case 'o':
                outfd = fopen(optarg, "w");

                if (outfd == NULL) {
                    perror("output");
                    return 1;
                }
                break;

            // choose syntax
            case 's':
                syntax = get_syntax(optarg);
                if (syntax < 0) {
                    fprintf(stderr, "Unrecognized syntax '%s'\n", optarg);
                    fprintf(stderr, "Supported syntax (-s) options:\n\n");
                    print_syntax_options();
                    return 1;
                }
                break;

            // choose bits
            case 'b':
                if (strcmp(optarg, "32") == 0) {
                    printf("\n\t32 bit is not ready!!\n");
                    printf("\n\t(use anyway? [y/N])\n");

                    if (getchar() != 'y')
                        return 0;

                    bits = KS_MODE_32; 
                    strcat(filename, "32");
                    filename[5] = 0;
                }
                else if (strcmp(optarg, "64") != 0)  {
                    fprintf(stderr, "Unsupported bits '%s'\n", optarg);
                    return 1;
                }
                break;

            // help
            case 'h':
                print_usage(argv[0]);
                return 0;

            // bad option
            default: 
                print_usage(argv[0]);
                return 1;
        }
    }

    // initialize assembler
    ks_engine *ks;
    if (ks_open(ARCH, bits, &ks) != KS_ERR_OK) {
        perror("ks_open");
        return 1;
    }

    // set syntax
    if (syntax != -1) {
        if (set_syntax(ks, syntax) < 0) {
            perror("set_syntax");
            return 1;
        }
    }

    // basic elf prelude 
    if (outfd != NULL && !no_prelude)
        fputs(ASMPRELUDE, outfd);

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
    pid_t child = run_trace(filename);

    // get status
    int status;
    wait(&status);
    if (WIFEXITED(status)) {
        puts("Failed to run subprocess");
        return 1;
    }

    // load blank instruction
    struct history *tmp, *curr;
    tmp = curr = malloc(sizeof(struct history));
    memset(curr->instruction, 0, MAXINSTRUCTIONSIZE);
    curr->next = curr->prev  = NULL;

    // initalise label list
    struct label *tmpl, *labels;
    tmpl = labels = NULL;

    // make first try all match
    struct user_regs_struct regsb, regsa;
    get_regs(child, &regsb);

    int addr_oset = 28;
    int y = 2;
    long long stack_pointer;
    long long inst_pointer;
    int ret = 0;
    while (1) {

        // clear and jump to top if at bottom
        if (y > LINES-3) {
            y = 2;
            wclear(instructions);
            box(instructions, '|' , '-');
            wrefresh(instructions);
        }

        // get register state 
        get_regs(child, &regsa);
        stack_pointer = regsa.rsp;
        inst_pointer = regsa.rip;

        // stack headings
        mvwprintw(stack, 2, width/7, "rsp");
        mvwprintw(stack, 1, width/2-3, "STACK");
        mvwprintw(stack, 2, (width)-width/5, "hexdump");

        // print stack, pointer, and hex dump
        print_stack(stack, child, stack_pointer, 2, 20, 19, 15);
        wrefresh(stack);

        // update registers
        mvwprintw(registers, 1, 11, "REGISTERS");
        print_regs(registers, 2, &regsb, &regsa);
        wrefresh(registers);

        // update flags 
        mvwprintw(registers, 1, width-20, "FLAGS");
        print_flags(registers, width-33, &regsb, &regsa);
        wrefresh(registers);

        // save register state for comparison
        get_regs(child, &regsb);

        // print current address (rip) 
        mvwprintw(instructions, y, addr_oset, "[%#010llx]> ", inst_pointer);
        wrefresh(instructions);

        // get instruction
        curr = get_instruction(instructions, curr, 42, y);

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
                    ret = 1;
                    break;
                }
                wait(&status);
            }
            if (ret) break;

            wmove(instructions, y, 40);
            clear_line(instructions);
        }

        else {
            // if last operand is a label, replace with address
            // ---------- ONLY WORKING FOR JUMPS -------------
            replace_label(labels, curr->instruction, inst_pointer);

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
                puttdata(child, inst_pointer, bytes, nbytes);

                // set process to break on step
                if (ptrace(PTRACE_SINGLESTEP, child, NULL, NULL) < 0) {
                    perror("Step Fail");
                    ret = 1;
                    break;
                }

                // try execute bytes
                wait(&status);

                // save to file
                if (outfd != NULL) 
                    fprintf(outfd, "\t%s\n", curr->instruction);
            }
            else {

                // set label at address
                if (is_label(curr->instruction)) {
                    wmove(instructions, y, 2);
                    clear_line(instructions);

                    tmpl = addlabel(labels, curr->instruction, inst_pointer);
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

    ks_close(ks);
    delwin(instructions);
    delwin(stack);
    delwin(registers);
    endwin();
    return ret;
}

int get_regs(pid_t child, struct user_regs_struct *regs) {
    return ptrace(PTRACE_GETREGS, child, 0, regs);
}

void print_usage(char *argv0) {
    fprintf(stderr, USAGE, argv0);
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


