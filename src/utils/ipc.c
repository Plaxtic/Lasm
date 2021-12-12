#ifndef SYSCALLS_H
#define SYSCALLS_H

#include <stddef.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <ctype.h>
#include <sys/ptrace.h>

#include "ipc.h"


long long gettdata(pid_t child, long long addr, char *str, int len) {
    long long word;
    long long read = 0;

    while (read < len) {
        word = ptrace(PTRACE_PEEKDATA, child, addr + read);
        read += LONGSIZ;
        
        if (read > len) {
            memcpy(str, &word, LONGSIZ - (read-len));
            break;
        }

        memcpy(str, &word, LONGSIZ);
        str += LONGSIZ;
    }

    return read;
}

void puttdata(pid_t child, long long addr, uint8_t *str, int len) {
    long long written = 0;

    while (written < len+LONGSIZ) {
        ptrace(PTRACE_POKEDATA, child, addr + written, *(long long *)str);
        written += LONGSIZ;
        str += LONGSIZ;
    }
}
#endif
