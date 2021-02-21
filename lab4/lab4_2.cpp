#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
    printf("lab4_2 parent's PID is: \t%d\n", getppid());
    printf("lab4_2 PID is: \t\t\t%d\n", getpid());

    pid_t pid { fork() };

    if(!pid) {
        printf("lab4_2 child's PID is: \t\t%d\n\n", getpid());

        execl("./lab4_1", "./lab4_1", "lab4_2's 1st msg", "lab4_2's 2nd msg", "lab4_2's 3rd msg", nullptr);
    }
    else {
        int sleepTime { 500000 },
            status    { 0 };
        
        while (!waitpid(pid, &status, WNOHANG))
            usleep(sleepTime);
        
        if (WIFEXITED(status))
            printf("\nChild process finished with code: %d\n", WEXITSTATUS(status));
    }

    return 0;
}
