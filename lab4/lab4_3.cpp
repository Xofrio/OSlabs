#include <sched.h>
#include <cstring>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

int child_function(void *arguments) {
    printf("lab4_3 child's PID is: \t\t%d\n\n", getpid());

    execl("./lab4_1",
              "./lab4_1",
              "lab4_3's 1st msg",
              "lab4_3's 2nd msg",
              "lab4_3's 3rd msg",
              nullptr);
    
    return 0;
}

int main(int argc, char *argv[]) {
    printf("lab4_3 parent's PID is: \t%d\n", getppid());
    printf("lab4_3 PID is: \t\t\t%d\n", getpid());

    size_t stackSize { 1048576 };
    char *stack     { 0 }, 
         *stackHead { 0 };
    
    stack = (char *)malloc(stackSize);
    stackHead = stack + stackSize - 1;

    int childPid { clone(&child_function, stackHead, SIGCHLD, argv) };
    
    if(childPid == -1) {
        printf("\nCouldn't clone. Error: %s\n", strerror(errno));

        free(stack);

        int exitCloneError { 22 };

        exit(exitCloneError);
    }

    int sleepTime { 500000 },
        status    { 0 };
    
    while (!waitpid(childPid, &status, WNOHANG))
        usleep(sleepTime);
        
    if (WIFEXITED(status))
        printf("\nChild process finished with code: %d\n", WEXITSTATUS(status));
    return 0;
}
