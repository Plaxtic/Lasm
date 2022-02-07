#ifndef SYSCALLS_H
#define SYSCALLS_H

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

int puttdata(pid_t child, long long addr, uint8_t *str, int len) {
    long long written = 0;
    int ret;

    while (written < len+LONGSIZ) {
        ret = ptrace(PTRACE_POKEDATA, child, addr + written, *(long long *)str);

        if (ret == -1) {
            return -1;
        }
        written += LONGSIZ;
        str += LONGSIZ;
    }
    return written;
}
#endif
