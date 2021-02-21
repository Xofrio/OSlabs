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
        ++buffer;
        if (buffer == 10)
            buffer = 1;

        ssize_t writeStatus { write(fileDescriptor, &buffer, sizeof(int)) };

        if (writeStatus == -1)
            printf("Couldn't write to buffer. Error: %s\n", strerror(errno));
        else
            printf("Writer message: %d\n", buffer);

        sleep(sleepTime);
    }
    pthread_exit(nullptr);
}

int main() {
    pthread_t thread;
    Accessory forThread { 1, true };

    mkfifo("/tmp/pipe", 0644);

    forThread.fileDescriptor = open("/tmp/pipe", O_WRONLY);
    
    if (pthread_create(&thread, nullptr, &thread_job, (void*)&forThread))
        perror("Failed to create thread in lab7_1.cpp.");

    getchar();

    forThread.flag = false;

    if (pthread_join(thread, nullptr))
        perror("Failed to join thread in lab7_1.cpp.");

    close(forThread.fileDescriptor);

    unlink("/tmp/pipe");

    return 0;
}
