#include "kernel/types.h"
#include "user.h"

int main(int argc, char** argv) {

    if (argc < 2) {
        printf("No syscall id provided");
        exit(1);
    }

    int syscall_id = atoi(argv[1]);
    trace(syscall_id);

    char *rgv[argc];
     for (int i = 0; i < argc-2; i++) {
        rgv[i] = argv[i+2];
    }
    rgv[argc-2] = 0;

    if (exec(argv[2], rgv) < 0) {
        printf("exec failed!\n");
    }

    exit(0);
}