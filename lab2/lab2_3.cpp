#include <pthread.h>
#include <stdio.h>
#include <unistd.h>

pthread_mutex_t mutex;

class Accessory {
    public:
        int numberOfSymbols,
            sleepTime;
        bool flag;
        char data;
        timespec time;

        Accessory(int numberOfSymbols, int sleepTime, bool flag, char data) {
            this->numberOfSymbols = numberOfSymbols;
            this->sleepTime = sleepTime;
            this->flag = flag;
            this->data = data;
        }

};

void *first_thread_job(void *information) {
    char symbol { *(&((Accessory*)information)->data) };
    int numberOfSymbols { *(&((Accessory*)information)->numberOfSymbols) },
        sleepTime       { *(&((Accessory*)information)->sleepTime) };
    
    while(*(&((Accessory*)information)->flag)) {
        clock_gettime(CLOCK_REALTIME, &((Accessory*)information)->time);
        *(&((Accessory*)information)->time.tv_sec) += 1;

        if(!pthread_mutex_timedlock(&mutex, &((Accessory*)information)->time)) {
            for (int i { 0 }; i < numberOfSymbols; ++i) {
                printf("%c", symbol);
                fflush(stdout);
            }
            pthread_mutex_unlock(&mutex);
            sleep(sleepTime);
        }
    }
    return nullptr;
}

void *second_thread_job(void *information) {
    char symbol { *(&((Accessory*)information)->data) };
    int numberOfSymbols { *(&((Accessory*)information)->numberOfSymbols) },
        sleepTime       { *(&((Accessory*)information)->sleepTime) };
    
    while(*(&((Accessory*)information)->flag)) {
        clock_gettime(CLOCK_REALTIME, &((Accessory*)information)->time);
        *(&((Accessory*)information)->time.tv_sec) += 1;

        if(!pthread_mutex_timedlock(&mutex, &((Accessory*)information)->time)) {
            for (int i { 0 }; i < numberOfSymbols; ++i) {
                printf("%c", symbol);
                fflush(stdout);
            }
            pthread_mutex_unlock(&mutex);
            sleep(sleepTime);
        }
    }
    return nullptr;
}

int main() {
    pthread_t thread1, 
              thread2;
    Accessory forFirstThread  { 5, 1, true, '1' },
              forSecondThread { 5, 1, true, '2' };
              
    pthread_mutex_init(&mutex, nullptr);

    if (pthread_create(&thread1, nullptr, &first_thread_job, (void*)&forFirstThread))
        perror("Failed to create thread 1.");
    if (pthread_create(&thread2, nullptr, &second_thread_job, (void*)&forSecondThread))
        perror("Failed to create thread 2.");

    getchar();

    forFirstThread.flag = forSecondThread.flag = false;

    if (pthread_join(thread1, nullptr))
        perror("Failed to join thread 1.");
    if (pthread_join(thread2, nullptr))
        perror("Failed to join thread 2.");

    pthread_mutex_destroy(&mutex);

    return 0;
}
