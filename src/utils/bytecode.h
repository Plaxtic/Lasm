#ifndef BYTECODE_H
#define BYTECODE_H

#include <sys/user.h>
#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>

#include "app.h"

#define MAXBYTECODESIZ 16

struct assembled_bytes {
    uint8_t bytecode[MAXBYTECODESIZ];
    size_t nbytes;
    struct assembled_bytes *next;
};

int add_bytecode(struct shell_context *, uint8_t*, size_t);
void print_n_instr_bytecode(struct shell_context *, int);
void copy_n_instr_bytecode(struct shell_context *, int);

#endif
