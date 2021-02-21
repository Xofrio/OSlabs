#include <cstring>
#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <stdio.h>
#include <unistd.h>

constexpr int numberOfDescriptors { 2 },
              writeDescriptor     { 1 },
              readDescriptor      { 0 };
int fileDescriptors[numberOfDescriptors] { 0 };

class Accessory {
    public:
        int sleepTime;
        bool flag;

        Accessory(int sleepTime, bool flag) {
            this->sleepTime = sleepTime;
            this->flag = flag;
        }
};

void *thread1_job(void *information) {
    int buffer    { 0 },
        sleepTime { *(&((Accessory*)information)->sleepTime) };
    bool *flag { (&((Accessory*)information)->flag) };

    while(*flag) {
        ++buffer;
        if (buffer == 10)
            buffer = 1;
        
        ssize_t writeStatus { write(fileDescriptors[writeDescriptor], &buffer, sizeof(int)) };

        if (writeStatus == -1)
            printf("Couldn't write to buffer. Error: %s\n", strerror(errno));
      
        sleep(sleepTime);
    }
    pthread_exit(NULL);
}

void *thread2_job(void *information) {
    int buffer    { 0 },
        sleepTime { *(&((Accessory*)information)->sleepTime) };
    bool *flag { (&((Accessory*)information)->flag) };

    while(*flag) {
        buffer = 0;

        ssize_t readStatus { read(fileDescriptors[readDescriptor], &buffer, sizeof(int)) };

        if (readStatus == -1)
            printf("Got no message from buffer. Error: %s\n", strerror(errno));
        else 
            printf("Message: %d\n", buffer);
        
        sleep(sleepTime);
    }
    pthread_exit(nullptr);
}

int main() {
    pthread_t thread1,
              thread2;
    Accessory forThread1 { 1, true },
              forThread2 { 1, true };

    pipe(fileDescriptors);
    pipe2(fileDescriptors, O_NONBLOCK);
    
    if (pthread_create(&thread1, nullptr, &thread1_job, (void*)&forThread1))
        perror("Failed to create thread 1.");
    if (pthread_create(&thread2, nullptr, &thread2_job, (void*)&forThread2))
        perror("Failed to create thread 2.");
    
    getchar();
    
    forThread1.flag = forThread2.flag = false;

    if (pthread_join(thread1, nullptr))
        perror("Failed to join thread 1.");
    if (pthread_join(thread2, nullptr))
        perror("Failed to join thread 2.");

    close(fileDescriptors[readDescriptor]);
    close(fileDescriptors[writeDescriptor]);

    return 0;
}
