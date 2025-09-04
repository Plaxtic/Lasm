#include <stddef.h>
#include <string.h>
#include <sys/ptrace.h>
#include <sys/wait.h>
#include <stdint.h>

#ifdef INSTALL
#define NULFILE "/usr/local/bin/nul"
#else
#define NULFILE "./nul"
#endif

#define MAXSYSCALLNAMELEN 22
#define MAXSYSCALLFORMATLEN MAXSYSCALLNAMELEN + (50*9) 
#define LONGSIZ sizeof(long long)

long long gettdata(pid_t, long long, char*, int);
int puttdata(pid_t, long long, uint8_t*, int);
