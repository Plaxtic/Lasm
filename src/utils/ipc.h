#define MAXSYSCALLNAMELEN 22
#define MAXSYSCALLFORMATLEN MAXSYSCALLNAMELEN + (50*9) 
#define LONGSIZ sizeof(long long)

#include <sys/wait.h>

long long gettdata(pid_t, long long, char*, int);
void puttdata(pid_t, long long, uint8_t*, int);

