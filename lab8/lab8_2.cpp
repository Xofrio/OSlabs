#include <cstring>
#include <errno.h>
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
    int sleepTime { *(&((Accessory*)information)->sleepTime) };
    bool *flag { &((Accessory*)information)->flag };

    while(*flag) {
        memset((&((Accessory*)information)->message)->buffer, 0, sizeof(&((Accessory*)information)->message));
        
        ssize_t readStatus { msgrcv(*(&((Accessory*)information)->queue), &((Accessory*)information)->message, sizeof((&((Accessory*)information)->message)->buffer), (&((Accessory*)information)->message)->type, IPC_NOWAIT) };

        if (readStatus == -1)
            printf("Got no message from buffer. Error: %s\n", strerror(errno));
        else
            printf((&((Accessory*)information)->message)->buffer ? "I got a message: %c\n" : "Got no message. Local data: %c\n" , *((&((Accessory*)information)->message)->buffer));

        sleep(sleepTime);
    }
    pthread_exit(nullptr);
}

int main() {
    pthread_t thread;
    Accessory forThread { 1, true, 1 };

    forThread.queue = msgget(ftok("/tmp", 'a'), IPC_CREAT | 0644);
    
    if (pthread_create(&thread, nullptr, &thread_job, (void*)&forThread))
        perror("Failed to create thread in lab8_2.cpp.");

    getchar();

    forThread.flag = false;

    if (pthread_join(thread, nullptr))
        perror("Failed to join thread in lab8_2.cpp.");

    msgctl(forThread.queue, IPC_RMID, nullptr);

    return 0;
}
