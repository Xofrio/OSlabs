#include <cstring>
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
    char buffer[bufferSize] { "1" };

    while(*flag) {
        int writeStatus { mq_send(*(&((Accessory*)information)->queue), buffer, strlen(buffer), 0) };

        if (writeStatus == -1)
            perror("Couldn't write to buffer. Error");
        else {
            printf("I wrote a message: %s\n", buffer);
            if (buffer[0] == '9')
                buffer[0] = '0';
            ++buffer[0];
        }

        sleep(sleepTime);
    }
    pthread_exit(nullptr);
}

int main() {
    pthread_t thread;
    Accessory forThread { 1, true };

    forThread.queue = mq_open("/queue", O_CREAT | O_WRONLY | O_NONBLOCK, 0644, nullptr);

    if (pthread_create(&thread, nullptr, &thread_job, (void*)&forThread))
        perror("Failed to create thread in lab8_11. Error");

    getchar();

    forThread.flag = false;

    if (pthread_join(thread, nullptr))
        perror("Failed to join thread in lab8_11. Error");
    
    mq_close(forThread.queue);
    mq_unlink("/queue");

    return 0;
}
