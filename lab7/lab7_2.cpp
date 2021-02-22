#include <cstring>
#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>

class Accessory {
    public:
        int sleepTime,
            fileDescriptor;
        bool flag;

        Accessory(int sleepTime, bool flag) {
            this->sleepTime = sleepTime;
            this->flag = flag;
        }
};

void *thread_job(void *information) {
    int buffer         { 0 },
        sleepTime      { *(&((Accessory*)information)->sleepTime) },
        fileDescriptor { *(&((Accessory*)information)->fileDescriptor) };
    bool *flag { &((Accessory*)information)->flag };

    while(*flag) {
        buffer = 0;

        ssize_t readStatus { read(fileDescriptor, &buffer, sizeof(int)) };

        if (readStatus == -1)
            printf("Got no message from buffer. Error: %s\n", strerror(errno));
        else
            printf(buffer ? "I got a message: %d\n" : "Got no message. Local data: %d\n" , buffer);

        sleep(sleepTime);
    }
    pthread_exit(nullptr);
}

int main() {
    pthread_t thread;
    Accessory forThread { 1, true };

    mkfifo("/tmp/pipe", 0644);
    
    forThread.fileDescriptor = open("/tmp/pipe", O_RDONLY | O_NONBLOCK);
    
    if (pthread_create(&thread, nullptr, &thread_job, (void*)&forThread))
        perror("Failed to create thread in lab7_2.cpp.");

    getchar();

    forThread.flag = false;

    if (pthread_join(thread, nullptr))
        perror("Failed to join thread in lab7_2.cpp.");

    close(forThread.fileDescriptor);

    unlink("/tmp/pipe");

    return 0;
}
