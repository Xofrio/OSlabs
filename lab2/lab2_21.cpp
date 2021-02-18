#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <unistd.h>

class Accessory { 
    public:
        int numberOfSymbols,
            sleepTime;
        bool flag;
        char data;

        Accessory(int numberOfSymbols, int sleepTime, bool flag, char data) {
            this->numberOfSymbols = numberOfSymbols;
            this->sleepTime = sleepTime;
            this->flag = flag;
            this->data = data;
        }
};

sem_t semaphore;

void *thread1_job(void *information) {
    int numberOfSymbols { *(&((Accessory*)information)->numberOfSymbols) },
        sleepTime       { *(&((Accessory*)information)->sleepTime) };
    char symbol { *(&((Accessory*)information)->data) };
    bool *flag { (&((Accessory*)information)->flag) };

    while(*flag) {
        if(!sem_trywait(&semaphore)) {
            for (int i { 0 }; i < numberOfSymbols; ++i) {
                printf("%c", symbol);
                fflush(stdout);

                sleep(sleepTime);
            }
            sem_post(&semaphore);

            sleep(sleepTime);
        }
    }
    return nullptr;
}

void *thread2_job(void *information) {
    int numberOfSymbols { *(&((Accessory*)information)->numberOfSymbols) },
        sleepTime       { *(&((Accessory*)information)->sleepTime) };
    char symbol { *(&((Accessory*)information)->data) };
    bool *flag { (&((Accessory*)information)->flag) };

    while(*flag) {
        if(!sem_trywait(&semaphore)) {
            for (int i { 0 }; i < numberOfSymbols; ++i) {
                printf("%c", symbol);
                fflush(stdout);

                sleep(sleepTime);
            }
            sem_post(&semaphore);
            
            sleep(sleepTime);
        }
    }
    return nullptr;
}

int main() {
    pthread_t thread1, 
              thread2;
    Accessory forThread1 { 5, 1, true, '1' },
              forThread2 { 5, 1, true, '2' };

    sem_init(&semaphore, 0, 1);

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

    sem_destroy(&semaphore);

    return 0;
}
