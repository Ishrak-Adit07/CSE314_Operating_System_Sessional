#include "kernel/types.h"
#include "user.h"

struct procInfo {
    int activeProcess;  // # of processes in RUNNABLE and RUNNING state
    int totalProcess;   // # of total possible processes
    int memsize;        // in bytes; summation of all active process
    int totalMemSize;   // in bytes; all available physical Memory
 };

int main(int argc, char** argv) {

    struct procInfo proc_info;
    info(&proc_info);

    printf("Current system information:\n");
    printf("Processes: %d/%d\n", proc_info.activeProcess, proc_info.totalProcess);
    unsigned long mem_mb = (proc_info.memsize * 1000) / (1024 * 1024);
    printf("RAM: %lu.%lu/%d (in MB)\n", mem_mb / 1000, mem_mb % 1000, proc_info.totalMemSize);

    exit(0);
}