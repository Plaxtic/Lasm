#include <linux/limits.h>

#include "utils/ipc.h"
#include "utils/history.h"
#include "utils/windows.h"
#include "utils/labels.h"
#include "utils/syntax.h"
#include "utils/wrappers.h"

int main(int argc, char **argv) {

    // default options
    int bits = KS_MODE_64;
    int syntax = -1;
    bool no_prelude = false;
    FILE *outfd = NULL;
#ifdef INSTALL
    char filename[22] = "/usr/local/bin/nul";
#else
    char filename[22] = "./nul";
#endif

    // get options
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
                    return EXIT_FAILURE;
                }
                break;

            // choose syntax
            case 's':
                syntax = get_syntax(optarg);
                if (syntax < 0) {
                    fprintf(stderr, "Unrecognized syntax '%s'\n", optarg);
                    fprintf(stderr, "Supported syntax (-s) options:\n\n");
                    print_syntax_options();
                    return EXIT_FAILURE;
                }
                break;

            // choose bits
            case 'b':
                if (strcmp(optarg, "32") == 0) {
                    printf("\n\t32 bit is not ready!!\n");
                    printf("\n\t(use anyway? [y/N])\n");

                    if (toupper(getchar()) != 'Y')
                        return EXIT_SUCCESS;

                    bits = KS_MODE_32; 
                    strcat(filename, "32");
                    filename[21] = 0;
                }
                else if (strcmp(optarg, "64") != 0)  {
                    fprintf(stderr, "Unsupported bits '%s'\n", optarg);
                    return EXIT_FAILURE;
                }
                break;

            // help
            case 'h':
                print_usage(argv[0]);
                return EXIT_SUCCESS;

            // bad option
            default: 
                print_usage(argv[0]);
                return EXIT_FAILURE;
        }
    }

    // initialize assembler
    ks_engine *ks;
    if (ks_open(ARCH, bits, &ks) != KS_ERR_OK) {
        perror("ks_open");
        return EXIT_FAILURE;
    }

    // set syntax
    if (syntax != -1) {
        if (set_syntax(ks, syntax) < 0) {
            perror("set_syntax");
            return EXIT_FAILURE;
        }
    }

    // basic elf prelude 
    if (outfd != NULL && !no_prelude)
        fputs(ASMPRELUDE, outfd);

    // run program in child process and trace with parent
    pid_t child = run_trace(filename);

    // get status
    int status;
    wait(&status);
    if (WIFEXITED(status)) {
        puts("Failed to run subprocess");
        return EXIT_FAILURE;
    }

    // load history log
#ifdef INSTALL
    char history_path[PATH_MAX];
    snprintf(history_path, PATH_MAX, "%s/.lasm_history", getenv("HOME"));
#else
    char history_path[] = ".lasm_history";
#endif
    FILE *log = fopen(history_path, "r+");
    if (log == NULL) {
        perror("fopen");
        return EXIT_FAILURE;
    }

    struct history *tmp, *curr;
    tmp = curr = load_history(log);
    fseek(log, 0, SEEK_END);

    // initalise label list
    struct label *tmpl, *labels;
    tmpl = labels = NULL;

    // fill registers to make first try all match
    struct user_regs_struct regsb, regsa;
    get_regs(child, &regsb);

    // init ncurses
    initscr();
    raw();
    noecho();
    keypad(stdscr, TRUE);
    if (LINES < 40 || COLS < 170) {
        fprintf(stderr, "Terminal is too small, continue? [N/y] ");

        if (toupper(getchar()) != 'Y') {
            endwin();
            return EXIT_SUCCESS;
        }
    }

    // set dimensions 
    int starty = (LINES - SUBWINHEIGHT);
    int startx = (COLS - SUBWINWIDTH);

    // create 3 windows
    WINDOW *registers = create_newwin(SUBWINHEIGHT, SUBWINWIDTH, starty, startx);
    WINDOW *stack = create_newwin(SUBWINHEIGHT, SUBWINWIDTH, 0, startx);
    WINDOW *instructions = create_newwin(MAINWINHEIGHT, MAINWINWIDTH, 0, 0);
    keypad(instructions, TRUE);

    // defaults
    int addr_oset = 28;
    int y = 2;
    int ret = EXIT_SUCCESS;

    // main loop
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
        long long stack_pointer = regsa.rsp;
        long long inst_pointer = regsa.rip;

        // stack headings
        mvwprintw(stack, 2, SUBWINWIDTH/7, "rsp");
        mvwprintw(stack, 1, SUBWINWIDTH/2-3, "STACK");
        mvwprintw(stack, 2, (SUBWINWIDTH)-SUBWINWIDTH/5, "hexdump");

        // print stack, pointer, and hex dump
        print_stack(stack, child, stack_pointer, 2, 20, 19, 15);
        wrefresh(stack);

        // update registers
        mvwprintw(registers, 1, 11, "REGISTERS");
        print_regs(registers, 2, &regsb, &regsa);
        wrefresh(registers);

        // update flags 
        mvwprintw(registers, 1, SUBWINWIDTH-20, "FLAGS");
        print_flags(registers, SUBWINWIDTH-33, &regsb, &regsa);
        wrefresh(registers);

        // save register state for comparison
        get_regs(child, &regsb);

        // print current address (rip) 
        mvwprintw(instructions, y, addr_oset, "[%#010llx]> ", inst_pointer);
        wrefresh(instructions);

        // get instruction
        curr = get_instruction(instructions, curr, 42, y);
        if (curr == NULL) break;
        curr->addr = inst_pointer;

        // skip enter or deleted line
        if (!curr->instruction[0]) continue;

        // if q, end loop
        else if (strcmp(curr->instruction, "q") == 0) break;

        // step special command
        else if (!strcmp(curr->instruction, "s") || 
                 !strncmp(curr->instruction, "s ", 2)) {
            int nsteps, i = 0;
            clear_line(instructions);

            // skip whitespace
            while (curr->instruction[++i] == ' ');

            // mininum one step
            if (!(nsteps = strtoul(&curr->instruction[i], NULL, 10))) nsteps = 1;

            // take "n" steps
            for (i = 0; i < nsteps; ++i) {
                if (ptrace(PTRACE_SINGLESTEP, child, NULL, NULL) < 0) {
                    perror("Step Fail");
                    ret = EXIT_FAILURE;
                    break;
                }
                wait(&status);
            }
            if (ret != 0) break;

            wmove(instructions, y, 40);
            clear_line(instructions);
        }

        // is instruction or label 
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

                // down one line
                y++;

                // inject raw assembled bytes to instruction pointer (rip)
                puttdata(child, inst_pointer, bytes, nbytes);

                // set process to break on step
                if (ptrace(PTRACE_SINGLESTEP, child, NULL, NULL) < 0) {
                    perror("Step Fail");
                    ret = EXIT_FAILURE;
                    break;
                }

                // try execute bytes
                wait(&status);

                // save to file
                if (outfd != NULL) 
                    fprintf(outfd, "\t%s\n", curr->instruction);

                fprintf(log, "%llu:%s\n", curr->addr, curr->instruction);
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

        // if you are in the middle of history, save command and preserve chain 
        if (curr->next != NULL) {
            tmp = curr;
            curr = find_head(curr);
            memset(curr->instruction, 0, MAXINSTRUCTIONSIZE);
            strncpy(curr->instruction, tmp->instruction, MAXINSTRUCTIONSIZE);
        }
        if (strcmp(curr->instruction, curr->prev->instruction))
            curr = add_to_history(curr);
    }
    if (outfd) fclose(outfd);

    ks_close(ks);
    fclose(log);
    delwin(instructions);
    delwin(stack);
    delwin(registers);
    endwin();
    return ret;
}
