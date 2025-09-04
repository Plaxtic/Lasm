#ifndef WRAPPERS_H
#define WRAPPERS_H

#include "sys/user.h"
#include "unistd.h"
#include "sys/wait.h"
#include "sys/ptrace.h"
#include "keystone/keystone.h"

#define ARCH KS_ARCH_X86
#define USAGE "Usage: %s\n\
\n\
\t-o <outfile>\n\
\t-s <asm syntax>\n\
\t-b <bits>\n\n"

void print_usage(char*);
pid_t run_trace(char*);
uint8_t *assemble(const char *, size_t*, ks_engine*);
int get_regs(pid_t child, struct user_regs_struct *regs);

#endif
