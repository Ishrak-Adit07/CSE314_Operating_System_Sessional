#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "riscv.h"
#include "spinlock.h"
#include "proc.h"
#include "syscall.h"
#include "defs.h"

struct procInfo;

// Fetch the uint64 at addr from the current process.
int
fetchaddr(uint64 addr, uint64 *ip)
{
  struct proc *p = myproc();
  if(addr >= p->sz || addr+sizeof(uint64) > p->sz) // both tests needed, in case of overflow
    return -1;
  if(copyin(p->pagetable, (char *)ip, addr, sizeof(*ip)) != 0)
    return -1;
  return 0;
}

// Fetch the nul-terminated string at addr from the current process.
// Returns length of string, not including nul, or -1 for error.
int
fetchstr(uint64 addr, char *buf, int max)
{
  struct proc *p = myproc();
  if(copyinstr(p->pagetable, buf, addr, max) < 0)
    return -1;
  return strlen(buf);
}

static uint64
argraw(int n)
{
  struct proc *p = myproc();
  switch (n) {
  case 0:
    return p->trapframe->a0;
  case 1:
    return p->trapframe->a1;
  case 2:
    return p->trapframe->a2;
  case 3:
    return p->trapframe->a3;
  case 4:
    return p->trapframe->a4;
  case 5:
    return p->trapframe->a5;
  }
  panic("argraw");
  return -1;
}

// Fetch the nth 32-bit system call argument.
void
argint(int n, int *ip)
{
  *ip = argraw(n);
}

// Retrieve an argument as a pointer.
// Doesn't check for legality, since
// copyin/copyout will do that.
void
argaddr(int n, uint64 *ip)
{
  *ip = argraw(n);
}

// Fetch the nth word-sized system call argument as a null-terminated string.
// Copies into buf, at most max.
// Returns string length if OK (including nul), -1 if error.
int
argstr(int n, char *buf, int max)
{
  uint64 addr;
  argaddr(n, &addr);
  return fetchstr(addr, buf, max);
}

// Prototypes for the functions that handle system calls.
extern uint64 sys_fork(void);
extern uint64 sys_exit(void);
extern uint64 sys_wait(void);
extern uint64 sys_pipe(void);
extern uint64 sys_read(void);
extern uint64 sys_kill(void);
extern uint64 sys_exec(void);
extern uint64 sys_fstat(void);
extern uint64 sys_chdir(void);
extern uint64 sys_dup(void);
extern uint64 sys_getpid(void);
extern uint64 sys_sbrk(void);
extern uint64 sys_sleep(void);
extern uint64 sys_uptime(void);
extern uint64 sys_open(void);
extern uint64 sys_write(void);
extern uint64 sys_mknod(void);
extern uint64 sys_unlink(void);
extern uint64 sys_link(void);
extern uint64 sys_mkdir(void);
extern uint64 sys_close(void);

// Task 1
extern uint64 sys_trace(void);

// Task 2
extern uint64 sys_info(void);

// An array mapping syscall numbers from syscall.h
// to the function that handles the system call.
static uint64 (*syscalls[])(void) = {
[SYS_fork]    sys_fork,
[SYS_exit]    sys_exit,
[SYS_wait]    sys_wait,
[SYS_pipe]    sys_pipe,
[SYS_read]    sys_read,
[SYS_kill]    sys_kill,
[SYS_exec]    sys_exec,
[SYS_fstat]   sys_fstat,
[SYS_chdir]   sys_chdir,
[SYS_dup]     sys_dup,
[SYS_getpid]  sys_getpid,
[SYS_sbrk]    sys_sbrk,
[SYS_sleep]   sys_sleep,
[SYS_uptime]  sys_uptime,
[SYS_open]    sys_open,
[SYS_write]   sys_write,
[SYS_mknod]   sys_mknod,
[SYS_unlink]  sys_unlink,
[SYS_link]    sys_link,
[SYS_mkdir]   sys_mkdir,
[SYS_close]   sys_close,

// Task 1
[SYS_trace]   sys_trace,

// Task 2
[SYS_info]    sys_info,
};

void
print_syscall_args(int syscall_id) {
  int arg0, arg1;
  uint64 addr0, addr1;
  int MAX_LENGTH = 256;
  char command0[MAX_LENGTH];
  char command1[MAX_LENGTH];

  switch (syscall_id) {
    case SYS_fork:
      break;
    case SYS_exit:
      argint(0, &arg0);
      printf("%d", arg0);
      break;
    case SYS_wait:
      argaddr(0, &addr0);
      printf("%p", (int*)addr0);
      break;
    case SYS_pipe:
      argaddr(0, &addr0);
      printf("%p", (void*)addr0);
      break;
    case SYS_read:
      argint(0, &arg0);
      argaddr(1, &addr0);
      argint(2, &arg1);
      printf("%d, %p, %d", arg0, (void*)addr0, arg1);
      break;
    case SYS_kill:
      argint(0, &arg0);
      printf("%d", arg0);
      break;
    case SYS_exec:
      argstr(0, command0, MAX_LENGTH);
      argaddr(1, &addr1);
      printf("%s, %p", command0, (void*)addr1);
      break;
    case SYS_fstat:
      argint(0, &arg0);
      argaddr(1, &addr0);
      printf("%d, %p", arg0, (void*)addr0);
      break;
    case SYS_chdir:
      argstr(0, command0, MAX_LENGTH);
      printf("%s", command0);
      break;
    case SYS_dup:
      argint(0, &arg0);
      printf("%d", arg0);
      break;
    case SYS_getpid:
      break;
    case SYS_sbrk:
      argint(0, &arg0);
      printf("%d", arg0);
      break;
    case SYS_sleep:
      argint(0, &arg0);
      printf("%d", arg0);
      break;
    case SYS_uptime:
      break;
    case SYS_open:
      argstr(0, command0, MAX_LENGTH);
      argint(1, &arg0);
      printf("%s, %d", command0, arg0);
      break;
    case SYS_write:
      argint(0, &arg0);
      argaddr(1, &addr0);
      argint(2, &arg1);
      printf("%d, %p, %d", arg0, (void*)addr0, arg1);
      break;
    case SYS_mknod:
      argstr(0, command0, MAX_LENGTH);
      argint(1, &arg0);
      argint(2, &arg1);
      printf("%s, %d, %d", command0, arg0, arg1);
      break;
    case SYS_unlink:
      argstr(0, command0, MAX_LENGTH);
      printf("%s", command0);
      break;
    case SYS_link:
      argstr(0, command0, MAX_LENGTH);
      argstr(1, command1, MAX_LENGTH);
      printf("%s, %s", command0, command1);
      break;
    case SYS_mkdir:
      argstr(0, command0, MAX_LENGTH);
      printf("%s", command0);
      break;
    case SYS_close:
      argint(0, &arg0);
      printf("%d", arg0);
      break;
    default:
      break;
  }
}

const char *syscall_names[] = {
    [SYS_fork]    = "fork",
    [SYS_exit]    = "exit",
    [SYS_wait]    = "wait",
    [SYS_pipe]    = "pipe",
    [SYS_read]    = "read",
    [SYS_kill]    = "kill",
    [SYS_exec]    = "exec",
    [SYS_fstat]   = "fstat",
    [SYS_chdir]   = "chdir",
    [SYS_dup]     = "dup",
    [SYS_getpid]  = "getpid",
    [SYS_sbrk]    = "sbrk",
    [SYS_sleep]   = "sleep",
    [SYS_uptime]  = "uptime",
    [SYS_open]    = "open",
    [SYS_write]   = "write",
    [SYS_mknod]   = "mknod",
    [SYS_unlink]  = "unlink",
    [SYS_link]    = "link",
    [SYS_mkdir]   = "mkdir",
    [SYS_close]   = "close",
};

const char* getSyscallName(int syscall_id) {
  return syscall_names[syscall_id];
}

void
syscall(void)
{
  int num;
  struct proc *p = myproc();

  num = p->trapframe->a7;
  // int first_argument = p->trapframe->a0;
  if(num > 0 && num < NELEM(syscalls) && syscalls[num]) {

    // Task 1
    int syscall_id = p->syscall_id;
    if(num > 0 && num < NELEM(syscalls) && num==syscall_id){
      printf("pid: %d", p->pid);
      printf(", ");

      printf("syscall: %s", getSyscallName(syscall_id));
      printf(", ");

      printf("args: ");
      printf("(");
      print_syscall_args(syscall_id);
      printf(")");
      printf(", ");
    }

    // Not Task 1
    // Use num to lookup the system call function for num, call it,
    // and store its return value in p->trapframe->a0
    p->trapframe->a0 = syscalls[num]();

    // Task 1
    if(num > 0 && num < NELEM(syscalls) && num==syscall_id){
      printf("return: %lu", p->trapframe->a0);
      printf("\n");
    }
  }
  else {
    printf("%d %s: unknown sys call %d\n",
            p->pid, p->name, num);
    p->trapframe->a0 = -1;
  }
}