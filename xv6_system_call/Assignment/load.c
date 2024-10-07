#include "kernel/types.h"
#include "user.h"

struct procInfo {
    int activeProcess;  // # of processes in RUNNABLE and RUNNING state
    int totalProcess;   // # of total possible processes
    int memsize;        // in bytes; summation of all active process
    int totalMemSize;   // in bytes; all available physical Memory
 };

int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("Usage: load <childCount> <allocationAmount>\n");
        exit(1);
    }

    int childCount = atoi(argv[1]);
    int allocationAmount = atoi(argv[2]);

    printf("Parent going to sleep.\n");

    for (int i = 0; i < childCount; i++) {
        int pid = fork();
        
        if (pid < 0) {
            printf("Fork failed!\n");
            exit(1);
        }

        if (pid == 0) {
            printf("Child is created.\n");

            char *allocated_memory = (char *)malloc(allocationAmount);
            if (allocated_memory == 0) {
                printf("Child failed to allocate memory.\n");
                exit(1);
            }
            
            printf("Child allocated %d byte.\n", allocationAmount);

            printf("Child going to sleep.\n");
            sleep(30);

            free(allocated_memory);
            exit(0);
        } else {
            int a = 100;
            wait(&a);
        }
    }

    sleep(30);

    printf("Parent wake up.\n");
    
    struct procInfo proc_info;
    info(&proc_info);

    printf("Current system information:\n");
    printf("Processes: %d/%d\n", proc_info.activeProcess, proc_info.totalProcess);
    unsigned long mem_mb = (proc_info.memsize * 1000) / (1024 * 1024);
    printf("RAM: %lu.%lu/%d (in MB)\n", mem_mb / 1000, mem_mb % 1000, proc_info.totalMemSize); 

    exit(0);
}