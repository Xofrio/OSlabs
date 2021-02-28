#include <mqueue.h>
#include <pthread.h>
#include <stdio.h>
#include <unistd.h>

constexpr int bufferSize  { 8192 };

class Accessory {
    public:
        int sleepTime;
        bool flag;
        mqd_t queue;

        Accessory(int sleepTime, bool flag) {
            this->sleepTime = sleepTime;
            this->flag = flag;
        }
};

void *thread_job(void *information) {
    int sleepTime { *(&((Accessory*)information)->sleepTime) };
    bool *flag { &((Accessory*)information)->flag };
    char buffer[bufferSize];

    while(*flag) {
        buffer[0] = { '\0' };

        ssize_t readStatus { mq_receive(*(&((Accessory*)information)->queue), buffer, bufferSize, 0) };

        if (readStatus == -1)
            perror("Got no message from buffer. Error");
        else
            printf(buffer ? "I got a message: %s\n" : "Got no message. Local data: %s\n" , buffer);

        sleep(sleepTime);
    }
    pthread_exit(nullptr);
}

int main() {
    pthread_t thread;
    Accessory forThread { 1, true };

    forThread.queue = mq_open("/queue", O_CREAT | O_RDONLY | O_NONBLOCK, 0644, nullptr);
    
    if (pthread_create(&thread, nullptr, &thread_job, (void*)&forThread))
        perror("Failed to create thread in lab8_21. Error");

    getchar();

    forThread.flag = false;

    if (pthread_join(thread, nullptr))
        perror("Failed to join thread in lab8_21. Error");

    mq_close(forThread.queue);
    mq_unlink("/queue");

    return 0;
}
