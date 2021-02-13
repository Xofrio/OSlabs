#include <cstring>
#include <errno.h>
#include <pthread.h>
#include <stdio.h>
#include <unistd.h>

constexpr int numberOfDescriptors { 2 },
              writeDescriptor     { 1 },
              readDescriptor      { 0 },
              sleepTime           { 1 };
bool flag1 { true },
     flag2 { true };
int fileDescriptors[numberOfDescriptors] { 0 };

void *first_thread_job(void *flag) {
    int buffer { 0 };

    while(*(bool*)flag) {
        ++buffer;
        if (buffer == 10)
            buffer = 1;
        
        ssize_t writeStatus { write(fileDescriptors[writeDescriptor], &buffer, sizeof(int)) };

        if (writeStatus == -1)
            printf("Couldn't write to buffer. Error: %s\n", strerror(errno));
      
        sleep(sleepTime);
    }
    pthread_exit(nullptr);
}

void *second_thread_job(void *flag) {
    int buffer { 0 };

    while(*(bool*)flag) {
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

    pipe(fileDescriptors);
    
    pthread_create(&thread1, nullptr, &first_thread_job, &flag1);
    pthread_create(&thread2, nullptr, &second_thread_job, &flag2);
    
    getchar();
    
    flag1 = flag2 = false;

    pthread_join(thread1, nullptr);
    pthread_join(thread2, nullptr);

    close(fileDescriptors[readDescriptor]);
    close(fileDescriptors[writeDescriptor]);

    return 0;
}
