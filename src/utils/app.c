#include "app.h"
#include "syntax.h"
#include "wrappers.h"
#include "windows.h"
#include "ipc.h"

#define MAXSTACKARGSLEN 20

void shell_cleanup(struct shell_context *ctx) {

    // cleanup lists
    free_history(ctx->history_head);
    free_labels(ctx->labels_head);

    // close
    ks_close(ctx->ks);
    fclose(ctx->log);
    if (ctx->outfd) 
        fclose(ctx->outfd);
    if (ctx->outbin) 
        fclose(ctx->outbin);

    // shut windows
    delwin(ctx->win_instructions);
    delwin(ctx->win_stack);
    delwin(ctx->win_registers);
    endwin();
}

int shell_init(struct shell_context *ctx, int argc, char **argv) {

    // zero out context 
    memset(ctx, 0, sizeof(struct shell_context));

    // defaults
    ctx->addr_offset = 28;
    ctx->current_line = 2;

    // default options
    int bits = KS_MODE_64;
    int syntax = -1;
    bool no_prelude = false;
    char filename[22] = NULFILE;
    char *stack_args = NULL;

    // get options
    int op;
    while ((op = getopt(argc, argv, "a:b:s:o:i:hp")) != -1) {
        switch (op) {

            // suppress prelude
            case 'p': 
                no_prelude = true;
                break;

            // pass argumens to stack
            case 'a':
                if (strlen(optarg) > MAXSTACKARGSLEN) {
                    fprintf(stderr, "too many arguments\n");
                    return -1;
                }
                stack_args = optarg;
                break;

            // output asm
            case 'o':
                ctx->outfd = fopen(optarg, "w");

                if (ctx->outfd == NULL) {
                    perror("output");
                    return -1;
                }
                break;

            // output binary
            case 'b':
                ctx->outbin = fopen(optarg, "w");

                if (ctx->outbin == NULL) {
                    perror("output");
                    return -1;
                }
                break;

            // choose syntax
            case 's':
                syntax = get_syntax(optarg);
                if (syntax < 0) {
                    fprintf(stderr, "Unrecognized syntax '%s'\n", optarg);
                    fprintf(stderr, "Supported syntax (-s) options:\n\n");
                    print_syntax_options();
                    return -1;
                }
                break;

            // choose bits
            case 'i':
                if (strcmp(optarg, "32") == 0) {
                    printf("\n\t32 bit is not ready!!\n");
                    printf("\n\t(use anyway? [y/N])\n");

                    if (toupper(getchar()) != 'Y')
                        return -1;

                    bits = KS_MODE_32; 
                    strcat(filename, "32");
                    filename[21] = 0;
                }
                else if (strcmp(optarg, "64") != 0)  {
                    fprintf(stderr, "Unsupported bits '%s'\n", optarg);
                    return -1;
                }
                break;

            // help
            case 'h':
                print_usage(argv[0]);
                return -1;

            // bad option
            default: 
                print_usage(argv[0]);
                return -1;
        }
    }

    // initialize assembler
    if (ks_open(ARCH, bits, &ctx->ks) != KS_ERR_OK) {
        perror("ks_open");
        return -1;
    }

    // set syntax
    if (syntax != -1) {
        if (set_syntax(ctx->ks, syntax) < 0) {
            perror("set_syntax");
            return -1;
        }
    }

    // basic elf prelude 
    if (ctx->outfd != NULL && !no_prelude) fputs(ASMPRELUDE, ctx->outfd);

    // run program in child process and trace with parent
    ctx->child = run_trace(filename, stack_args);
    if (ctx->child == -1) {
        perror("run trace");
        return -1;
    }

    // get status
    wait(&ctx->child_status);
    if (WIFEXITED(ctx->child_status) || WIFSIGNALED(ctx->child_status)) {
        puts("Failed to run subprocess");
        return -1;
    }

    // open logfile
    ctx->log = get_history_file();
    if (ctx->log == NULL) {
        perror("Failed to load history file");
        return -1;
    }

    ctx->history_head = load_history(ctx->log);
    if (ctx->history_head == NULL) {
        perror("Failed to load history");
        fclose(ctx->log);
        ks_close(ctx->ks);
        if (ctx->outfd) fclose(ctx->outfd);
        return -1;
    }
    fseek(ctx->log, 0, SEEK_END);

    // initialise ncurses
    init_ncurses();
    int starty = (LINES - SUBWINHEIGHT);
    int startx = (COLS - SUBWINWIDTH);

    // create 3 windows
    ctx->win_stack = create_newwin(SUBWINHEIGHT, SUBWINWIDTH, 0, startx);
    ctx->win_instructions = create_newwin(MAINWINHEIGHT, MAINWINWIDTH, 0, 0);
    ctx->win_registers = create_newwin(SUBWINHEIGHT, SUBWINWIDTH, starty, startx);

    keypad(ctx->win_instructions, TRUE);

    if (get_regs(ctx->child, &ctx->regs_before) == -1) {
        perror("Failed to get registers");
        shell_cleanup(ctx);
        return -1;
    }
    ctx->regs_after = ctx->regs_before;

    return 0;
}

