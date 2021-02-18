#include <pthread.h>
#include <stdio.h>
#include <unistd.h>

class Accessory { 
    public:
        int numberOfSymbols,
            sleepTime;
        bool flag;
        char data;

        static pthread_mutex_t mutex;

        Accessory(int numberOfSymbols, int sleepTime, bool flag, char data) {
            this->numberOfSymbols = numberOfSymbols;
            this->sleepTime = sleepTime;
            this->flag = flag;
            this->data = data;
        }
};

void *thread1_job(void *information) {
    int numberOfSymbols { *(&((Accessory*)information)->numberOfSymbols) },
        sleepTime       { *(&((Accessory*)information)->sleepTime) };
    char symbol { *(&((Accessory*)information)->data) };
    bool *flag { (&((Accessory*)information)->flag) };

    while(*flag) {
        pthread_mutex_lock(&Accessory::mutex);

        for (int i { 0 }; i < numberOfSymbols; ++i) {
            printf("%c", symbol);
            fflush(stdout);

            sleep(sleepTime);
        }
        pthread_mutex_unlock(&Accessory::mutex);

        sleep(sleepTime);
    }
    return nullptr;
}

void *thread2_job(void *information) {
    int numberOfSymbols { *(&((Accessory*)information)->numberOfSymbols) },
        sleepTime       { *(&((Accessory*)information)->sleepTime) };
    char symbol { *(&((Accessory*)information)->data) };
    bool *flag { (&((Accessory*)information)->flag) };

    while(*flag) {
        pthread_mutex_lock(&Accessory::mutex);

        for (int i { 0 }; i < numberOfSymbols; ++i) {
            printf("%c", symbol);
            fflush(stdout);

            sleep(sleepTime);
        }
        pthread_mutex_unlock(&Accessory::mutex);

        sleep(sleepTime);
    }
    return nullptr;
}

pthread_mutex_t Accessory::mutex ( PTHREAD_MUTEX_INITIALIZER );

int main() {
    pthread_t thread1, 
              thread2;
    Accessory forThread1 { 5, 1, 1, '1' },
              forThread2 { 5, 1, 1, '2' };

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

    return 0;
}
