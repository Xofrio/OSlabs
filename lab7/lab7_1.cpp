#include <fcntl.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>

class Accessory {
    public:
        int sleepTime,
            fileDescriptor;
        bool flag;
        pthread_t thread;

        Accessory(int sleepTime, bool flag) {
            this->sleepTime = sleepTime;
            this->flag = flag;
        }
};

void signal_handler(int signalNumber) {
    printf("I managed to intercept SIGPIPE signal.\n"
           "If you terminanated reader not with Enter, you can try to launch reader again.\n"
           "Press Enter to terminate this program with a 5 seconds delay.\n");
    
    sleep(5);
}

void *transfer(void *information) {
    int buffer         { 0 },
        sleepTime      { *(&((Accessory*)information)->sleepTime) },
        fileDescriptor { *(&((Accessory*)information)->fileDescriptor) };
    bool *flag { &((Accessory*)information)->flag };

    while(*flag) {
        ++buffer;
        if (buffer == 10)
            buffer = 1;

        ssize_t writeStatus { write(fileDescriptor, &buffer, sizeof(int)) };

        if (writeStatus == -1)
            perror("Couldn't write to buffer. Error");
        else
            printf("I wrote a message: %d\n", buffer);

        sleep(sleepTime);
    }
    pthread_exit(nullptr);
}

void *open_(void *information) {
    int sleepTime { *(&((Accessory*)information)->sleepTime) };
    bool *flag { &((Accessory*)information)->flag };

    while(*flag) {
        *(&((Accessory*)information)->fileDescriptor) = open("/tmp/pipe", O_WRONLY | O_NONBLOCK);

        if (*(&((Accessory*)information)->fileDescriptor) < 0) {
            perror("Couldn't open file for writing. Error");

            sleep(sleepTime);
        }
        else {
            if (pthread_create(&((Accessory*)information)->thread, nullptr, &transfer, information))
                perror("Failed to create transfering thread in lab7_1. Error");

            pthread_exit(nullptr);
        }
    }
    pthread_exit(nullptr);
}

int main() {
    pthread_t thread;
    Accessory forThread { 1, true };

    signal(SIGPIPE, &signal_handler);

    mkfifo("/tmp/pipe", 0644);
    
    if (pthread_create(&thread, nullptr, &open_, (void*)&forThread))
        perror("Failed to create opening thread in lab7_1. Error");

    getchar();

    forThread.flag = false;

    if (pthread_join(thread, nullptr))
        perror("Failed to join opening thread in lab7_1. Error");
    if (pthread_join(forThread.thread, nullptr))
        perror("Failed to join transfering thread in lab7_1. Error");

    close(forThread.fileDescriptor);
    unlink("/tmp/pipe");

    return 0;
}
