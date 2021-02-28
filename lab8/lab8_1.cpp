#include <mqueue.h>
#include <pthread.h>
#include <stdio.h>
#include <sys/msg.h>
#include <unistd.h>

constexpr int bufferSize  { 256 };

class Accessory {
    public:
        int sleepTime;
        bool flag;
        mqd_t queue;
        
        struct Message {
            long type;
            char buffer[bufferSize];
        };
        
        Message message;

        Accessory(int sleepTime, bool flag, long type) {
            this->sleepTime = sleepTime;
            this->flag = flag;
            this->message.type = type;
        }
};

void *thread_job(void *information) {
    int counter   { 1 },
        sleepTime { *(&((Accessory*)information)->sleepTime) };
    bool *flag { &((Accessory*)information)->flag };

    while(*flag) {
        int messageSize { sprintf((&((Accessory*)information)->message)->buffer, "%d", counter) },
            writeStatus { msgsnd(*(&((Accessory*)information)->queue), (&((Accessory*)information)->message), messageSize, IPC_NOWAIT) };

        if (writeStatus == -1)
            perror("Couldn't write to buffer. Error");
        else {
            printf("I wrote a message: %c\n", *((&((Accessory*)information)->message)->buffer));
            if (counter == 9)
                counter = 0;
            ++counter;
        }

        sleep(sleepTime);
    }
    pthread_exit(nullptr);
}

int main() {
    pthread_t thread;
    Accessory forThread { 1, true, 1 };

    forThread.queue = msgget(ftok("/tmp", 'a'), IPC_CREAT | 0644);

    if (pthread_create(&thread, nullptr, &thread_job, (void*)&forThread))
        perror("Failed to create thread in lab8_1. Error");

    getchar();

    forThread.flag = false;

    if (pthread_join(thread, nullptr))
        perror("Failed to join thread in lab8_1. Error");
    
    msgctl(forThread.queue, IPC_RMID, nullptr);

    return 0;
}
