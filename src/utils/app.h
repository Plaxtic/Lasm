#ifndef APP_H
#define APP_H

#include <ncurses.h>
#include <keystone/keystone.h>
#include <sys/user.h>
#include "history.h"
#include "labels.h"

#define MAXBYTECODESIZ 16

struct shell_context {
    pid_t child;
    ks_engine *ks;
    FILE *log;
    FILE *outfd;
    FILE *outbin;
    struct history *history_head;
    struct assembled_bytes *bytecode_head;
    struct label *labels_head;
    struct user_regs_struct regs_before;
    struct user_regs_struct regs_after;
    WINDOW *win_instructions;
    WINDOW *win_stack;
    WINDOW *win_registers;
    int child_status;
    int addr_offset;
    int current_line;
    int should_quit;
};

// initialization + cleanup
int shell_init(struct shell_context *ctx, int argc, char **argv);
void shell_cleanup(struct shell_context *ctx);


#endif
