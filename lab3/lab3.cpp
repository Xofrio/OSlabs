//#include <fcntl.h>
#include <pthread.h>
#include <stdio.h>
#include <unistd.h>

constexpr int numberOfDescriptors { 2 },
              writeDescriptor     { 1 },
              readDescriptor      { 0 },
              sleepTime           { 1 };
bool flag1 { true },
     flag2 { true };
int fileDescriptors[numberOfDescriptors] {};

void *first_thread_job(void *flag) {
    int buffer { 0 };

    while(*(bool*)flag) {
        ++buffer;
        if (buffer == 10)
            buffer = 1;
        
        write(fileDescriptors[writeDescriptor], &buffer, sizeof(int));

        sleep(sleepTime);
    }
    pthread_exit(NULL);
}

void *second_thread_job(void *flag) {
    int buffer { 0 };

    while(*(bool*)flag) {
        buffer = 0;

        read(fileDescriptors[readDescriptor], &buffer, sizeof(int));
        printf("Message: %d\n", buffer);

        sleep(sleepTime);
    }
    pthread_exit(NULL);
}

int main() {
    pthread_t thread1,
              thread2;

    pipe(fileDescriptors);
    //pipe2(fileDescriptors, O_NONBLOCK);
    //fcntl(fileDescriptors, F_SETFL, O_NONBLOCK);
    
    pthread_create(&thread1, NULL, &first_thread_job, &flag1);
    pthread_create(&thread2, NULL, &second_thread_job, &flag2);
    
    getchar();
    
    flag1 = flag2 = false;

    pthread_join(thread1, NULL);
    pthread_join(thread2, NULL);

    close(fileDescriptors[readDescriptor]);
    close(fileDescriptors[writeDescriptor]);

    return 0;
}
