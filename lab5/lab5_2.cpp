#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>

class Accessory {
    public:
        int numberOfSymbols,
            sleepTime;
        bool flag;
        char data;
        FILE *file;
        sem_t *semaphore;

        Accessory(int numberOfSymbols, int sleepTime, bool flag, char data, FILE *file, sem_t *semaphore) {
            this->numberOfSymbols = numberOfSymbols;
            this->sleepTime = sleepTime;
            this->flag = flag;
            this->data = data;
            this->file = file;
            this->semaphore = semaphore;
        }
};

void *thread_job(void *information) {
    char symbol { *(&((Accessory*)information)->data) };
    int numberOfSymbols { *(&((Accessory*)information)->numberOfSymbols) },
        sleepTime       { *(&((Accessory*)information)->sleepTime) };
    bool *flag { &((Accessory*)information)->flag };
    FILE *file { *(&((Accessory*)information)->file) };
    sem_t *semaphore { *(&((Accessory*)information)->semaphore) };

    while(*flag) {
        if(!sem_wait(semaphore)) {
            for (int i { 0 }; i < numberOfSymbols; ++i) {
                printf("%c", symbol);
                fflush(stdout);
                
                fputc(symbol, file);
                fflush(file);                
            }
            sem_post(semaphore);

            sleep(sleepTime);
        }
    }
    return nullptr;
}

int main() {
    pthread_t thread;
    Accessory forThread { 5, 1, true, '2', fopen("lab5.txt", "a+"), sem_open("/semaphore", O_CREAT, 0644, 1)};

    if (pthread_create(&thread, nullptr, &thread_job, (void*)&forThread))
        perror("Failed to create thread in lab5_2.cpp.");

    getchar();

    forThread.flag = false;

    if (pthread_join(thread, nullptr))
        perror("Failed to join thread in lab5_2.cpp.");

    close(*(int*)forThread.file);

    sem_close(forThread.semaphore);
    sem_unlink("/semaphore");

    return 0;
}
