#define MAXSYSCALLNAMELEN 22
#define MAXSYSCALLFORMATLEN MAXSYSCALLNAMELEN + (50*9) 
#define LONGSIZ sizeof(long long)

#include <sys/wait.h>
struct syscall {
    int code;
    char *name;
    int nargs;
    char dref[6];
};

long long gettdata(pid_t, long long, char*, int);
extern struct syscall syscall_table[];

void puttdata(pid_t, long long, uint8_t*, int);
char *find_syscall_symbol(int);
int get_syscall_format_string(int, char[MAXSYSCALLFORMATLEN], long long[6], pid_t);

