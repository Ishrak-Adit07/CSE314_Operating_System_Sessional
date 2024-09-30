#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "proc.h"

uint64
sys_exit(void)
{
  int n;
  argint(0, &n);
  exit(n);
  return 0;  // not reached
}

uint64
sys_getpid(void)
{
  return myproc()->pid;
}

uint64
sys_fork(void)
{
  return fork();
}

uint64
sys_wait(void)
{
  uint64 p;
  argaddr(0, &p);
  return wait(p);
}

uint64
sys_sbrk(void)
{
  uint64 addr;
  int n;

  argint(0, &n);
  addr = myproc()->sz;
  if(growproc(n) < 0)
    return -1;
  return addr;
}

uint64
sys_sleep(void)
{
  int n;
  uint ticks0;

  argint(0, &n);
  if(n < 0)
    n = 0;
  acquire(&tickslock);
  ticks0 = ticks;
  while(ticks - ticks0 < n){
    if(killed(myproc())){
      release(&tickslock);
      return -1;
    }
    sleep(&ticks, &tickslock);
  }
  release(&tickslock);
  return 0;
}

uint64
sys_kill(void)
{
  int pid;

  argint(0, &pid);
  return kill(pid);
}

// return how many clock tick interrupts have occurred
// since start.
uint64
sys_uptime(void)
{
  uint xticks;

  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}

// Task 1
uint64
sys_trace(void)
{

  int syscall_id;
  argint(0, &syscall_id);
  // printf("%d\n", syscall_id);

  myproc()->syscall_id = syscall_id;

  // uint64 addr;
  // argaddr(0, &addr);

  // int *userspace_pointer = (int *)addr;
  // *userspace_pointer = 34;
  // printf("%lu\n", addr);

  // int user_value;
  // copyin(myproc()->pagetable, (char *)&user_value, addr, sizeof(int));
  // printf("Kernel received: %d\n", user_value);

  // user_value = 500;
  // copyout(myproc()->pagetable, addr, (char *)&user_value, sizeof(int));

  return 0;
}

struct procInfo {
  int activeProcess;  // # of processes in RUNNABLE and RUNNING state
  int totalProcess;   // # of total possible processes
  int memsize;        // in bytes; summation of all active process
  int totalMemSize;   // in bytes; all available physical Memory
};

extern void get_proc_info(int *activeProcess, int *totalProcess, int *memsize, int *totalMemSize);

// Task 2
uint64
sys_info(void)
{

  uint64 addr;
  argaddr(0, &addr);

  struct procInfo proc_info;
  copyin(myproc()->pagetable, (char *)&proc_info, addr, sizeof(struct procInfo));

  // proc_info.activeProcess = 1010;
  // proc_info.totalProcess = 1020;
  // proc_info.memsize = 2000;
  // proc_info.totalMemSize = 5000;
  copyout(myproc()->pagetable, addr, (char *)&proc_info, sizeof(struct procInfo));

  get_proc_info(&proc_info.activeProcess, &proc_info.totalProcess, &proc_info.memsize, &proc_info.totalMemSize);
  copyout(myproc()->pagetable, addr, (char *)&proc_info, sizeof(struct procInfo));

  return 100;
}