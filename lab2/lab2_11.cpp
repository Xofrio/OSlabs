#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <unistd.h>

constexpr int numberOfSymbols { 5 },
              sleepTime       { 1 };
bool flag1 { true },
     flag2 { true };
sem_t semaphore;

void *first_thread_job(void *flag) {
    while(*(bool*)flag) {
        sem_wait(&semaphore);

        for (int i { 0 }; i < numberOfSymbols; ++i) {
            printf("1");
            fflush(stdout);
        }
        sem_post(&semaphore);
        sleep(sleepTime);
    }
    return nullptr;
}

void *second_thread_job(void *flag) {
    while(*(bool*)flag) {
        sem_wait(&semaphore);

        for (int i { 0 }; i < numberOfSymbols; ++i) {
            printf("2");
            fflush(stdout);
        }
        sem_post(&semaphore);
        sleep(sleepTime);
    }
    return nullptr;
}

int main() {
    pthread_t thread1, 
              thread2;

    sem_init(&semaphore, 0, 1);

    if (pthread_create(&thread1, nullptr, &first_thread_job, &flag1))
        perror("Failed to create thread 1.");
    if (pthread_create(&thread2, nullptr, &second_thread_job, &flag2))
        perror("Failed to create thread 2.");

    getchar();

    flag1 = flag2 = false;

    if (pthread_join(thread1, nullptr))
        perror("Failed to join thread 1.");
    if (pthread_join(thread2, nullptr))
        perror("Failed to join thread 2.");

    sem_destroy(&semaphore);

    return 0;
}
