#include <stddef.h>

#include "command_parse.h"
#include "ipc.h"
#include "wrappers.h"

bool is_buggy_char(const char *instruction) {
    if (!instruction || !instruction[0]) 
        return false;

    // keystone hangs forver on :
    if (instruction[0] == ':') 
        return true;

    return false;
}

int handle_loops(struct shell_context *ctx,  unsigned long long *loop_begin) {
    long long inst_pointer = ctx->regs_after.rip;
    struct history *curr = ctx->history_head;

    if (*loop_begin != 0) {
        sleep(1);
        if (ptrace(PTRACE_SINGLESTEP, ctx->child, NULL, NULL) < 0) {
            perror("Step Fail");
            return 1;
        }
        wait(&ctx->child_status);

        if (inst_pointer > *loop_begin+1) {
            *loop_begin = 0;
        }
        return 1;
    }
    if (ctx->current_line > 3 && inst_pointer < curr->prev->addr) {
        *loop_begin = inst_pointer;
        return 1;
    }
    return 0;
}


int process_instruction(struct shell_context *ctx) {
    struct history *curr = ctx->history_head;

    // skip empty input
    if (!curr->instruction[0]) return 0;

    // handle quit command
    if (strcmp(curr->instruction, "q") == 0) return 1;

    // handle step command
    if (strcmp(curr->instruction, "s") == 0 ||
        strncmp(curr->instruction, "s ", 2) == 0) {
        return handle_step_command(ctx);
    }

    // handle assembly instructions and labels
    return handle_assembly(ctx);
}


int handle_step_command(struct shell_context *ctx) {
    struct history *curr = ctx->history_head;
    int nsteps, i = 0;
    clear_line(ctx->win_instructions);

    // skip whitespace
    while (curr->instruction[++i] == ' ');

    // get number of steps
    nsteps = strtoul(&curr->instruction[i], NULL, 10);
    if (nsteps == 0) nsteps = 1;

    // execute steps
    for (i = 0; i < nsteps; ++i) {
        if (ptrace(PTRACE_SINGLESTEP, ctx->child, NULL, NULL) < 0) {
            perror("Step Fail");
            return 1;
        }
        wait(&ctx->child_status);
    }
    wmove(ctx->win_instructions, ctx->current_line, 40);
    clear_line(ctx->win_instructions);
    return 0;
}

int handle_assembly(struct shell_context *ctx) {
    struct history *curr = ctx->history_head;

    // try to replace labels using ctx->labels_head
    replace_label(ctx->labels_head, curr->instruction, ctx->regs_after.rip);

    // try to assemble
    size_t nbytes = 0;
    uint8_t *bytes = NULL;
    if (!is_buggy_char(curr->instruction)) {
        bytes = assemble(curr->instruction, &nbytes, ctx->ks);
    }

    // try to execute
    if (nbytes > 0 && bytes != NULL) {
        return execute_assembly(ctx, bytes, nbytes);
    } else {
        return handle_label_creation(ctx);
    }
}


int execute_assembly(struct shell_context *ctx, uint8_t *bytes, size_t nbytes) {

    // cover "Invalid" with spaces
    struct history *curr = ctx->history_head;
    mvwprintw(ctx->win_instructions, ctx->current_line, 2, "           ");

    // print bytecode in hex
    for (int i = 0; i < nbytes; i++) {
        mvwprintw(ctx->win_instructions, ctx->current_line, 2 + (i * 2), "%02x", bytes[i]);
    }

    // move to next line
    ctx->current_line++;

    // inject raw assembled bytes to instruction pointer
    if (puttdata(ctx->child, ctx->regs_after.rip, bytes, nbytes) < 0) {
        perror("Failed to write to process memory");
        return -1;
    }

    // set process to break on step
    if (ptrace(PTRACE_SINGLESTEP, ctx->child, NULL, NULL) < 0) {
        perror("Step Fail");
        return -1;
    }

    // try execute
    wait(&ctx->child_status);

    // save to file
    if (ctx->outfd != NULL) {
        fprintf(ctx->outfd, "\t%s\n", curr->instruction);
    }

    fprintf(ctx->log, "%llu:%s\n", curr->addr, curr->instruction);

    return 0;
}

int handle_label_creation(struct shell_context *ctx) {
    struct history *curr = ctx->history_head;

    wmove(ctx->win_instructions, ctx->current_line, 2);
    clear_line(ctx->win_instructions);

    if (is_label(curr->instruction)) {
        struct label *new_label = addlabel(ctx->labels_head, curr->instruction, ctx->regs_after.rip);
        if (new_label == NULL) {
            mvwprintw(ctx->win_instructions, ctx->current_line, 2, "Bad Label!");
        } else {
            ctx->labels_head = new_label;
            mvwprintw(ctx->win_instructions, ctx->current_line, 2, "%s:", new_label->name);
            ctx->current_line++;
        }
    } else {
        mvwprintw(ctx->win_instructions, ctx->current_line, 2, "Invalid");
        clear_line(ctx->win_instructions);
    }
    return 0;
}

void manage_history(struct shell_context *ctx) {
    struct history *curr = ctx->history_head;

    if (curr->next != NULL) {
        struct history *tmp = curr;
        curr = find_head(curr);
        memset(curr->instruction, 0, MAXINSTRUCTIONSIZE);
        strncpy(curr->instruction, tmp->instruction, MAXINSTRUCTIONSIZE);
    }
    ctx->history_head = add_to_history(curr);
}
