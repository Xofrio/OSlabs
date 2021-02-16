#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
    if (argc < 4 || argc > 6)
        printf("Input 3-5 arguments in console when calling this program.\n");
    else {
        int sleepTime { 1 };

        for (int i { 1 }; i < argc; ++i) {
            printf("lab4_1 argument #%d: %s\n", i, argv[i]);
            sleep(sleepTime);
        }
        printf("\n");
    }

    printf("lab4_1 PID is: \t\t\t%d\n", getpid());
    printf("lab4_1 parent's PID is: \t%d\n", getppid());

    pid_t pid;
    int exitCode1 { 12 },
        exitCode2 { 32 };

    if (!pid)
        exit(exitCode1);
    else
        exit(exitCode2);
}
