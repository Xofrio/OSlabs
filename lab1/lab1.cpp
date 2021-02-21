#include <pthread.h>
#include <stdio.h>
#include <unistd.h>

constexpr int code1 { 6 },
              code2 { 9 };

class Accessory {
    public:
        int sleepTime;
        bool flag;
        char data;

        Accessory(int sleepTime, bool flag, char data) {
            this->sleepTime = sleepTime;
            this->flag = flag;
            this->data = data;
        }
};

void *thread1_job(void *information) {
    int sleepTime { *(&((Accessory*)information)->sleepTime) };
    bool *flag { &((Accessory*)information)->flag };
    char symbol { *(&((Accessory*)information)->data) };

    while(*flag) {
        printf("%c", symbol);
        fflush(stdout);
      
        sleep(sleepTime);
    }
    pthread_exit((void*)code1);
}

void *thread2_job(void *information) {
    int sleepTime { *(&((Accessory*)information)->sleepTime) };
    bool *flag { &((Accessory*)information)->flag };
    char symbol { *(&((Accessory*)information)->data) };

    while(*flag) {
        printf("%c", symbol);
        fflush(stdout);
      
        sleep(sleepTime);
    }
    pthread_exit((void*)code2);
}

int main() {
    pthread_t thread1, 
              thread2;
    Accessory forThread1 { 1, true, '1' },
              forThread2 { 1, true, '2' };

    if (pthread_create(&thread1, nullptr, &thread1_job, (void*)&forThread1))
        perror("Failed to create thread 1.");
    if (pthread_create(&thread2, nullptr, &thread2_job, (void*)&forThread2))
        perror("Failed to create thread 2.");

    getchar();

    forThread1.flag = forThread2.flag = false;

    int result1 { 0 },
        result2 { 0 };

    if (pthread_join(thread1, nullptr))
        perror("Failed to join thread 1.");
    if (pthread_join(thread2, nullptr))
        perror("Failed to join thread 2.");

    printf("Thread 1 return code: %d.\n", result1);
    printf("Thread 2 return code: %d.\n", result2);

    return 0;
}
