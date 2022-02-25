#ifndef WRAPPERS_H
#define WRAPPERS_H

#include "wrappers.h"

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
#endif
