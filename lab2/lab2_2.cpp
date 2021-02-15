#include <pthread.h>
#include <stdio.h>
#include <unistd.h>

constexpr int numberOfSymbols { 5 },
              sleepTime       { 1 };
bool flag1 { true },
     flag2 { true };
pthread_mutex_t mutex;

void *first_thread_job(void *flag) {
    while(*(bool*)flag) {
        if(!pthread_mutex_trylock(&mutex)) {
            for (int i { 0 }; i < numberOfSymbols; ++i) {
                printf("1");
                fflush(stdout);
            }
            pthread_mutex_unlock(&mutex);
            sleep(sleepTime);
        }
    }
    return NULL;
}

void *second_thread_job(void *flag) {
    while(*(bool*)flag) {
        if(!pthread_mutex_trylock(&mutex)) {
            for (int i { 0 }; i < numberOfSymbols; ++i) {
                printf("2");
                fflush(stdout);
            }
            pthread_mutex_unlock(&mutex);
            sleep(sleepTime);
        }
    }
    return NULL;
}

int main() {
    pthread_t thread1, 
              thread2;

    pthread_mutex_init(&mutex, NULL);

    if (pthread_create(&thread1, NULL, &first_thread_job, &flag1))
        perror("Failed to create thread 1.");
    if (pthread_create(&thread2, NULL, &second_thread_job, &flag2))
        perror("Failed to create thread 2.");

    getchar();

    flag1 = flag2 = false;

    if (pthread_join(thread1, NULL))
        perror("Failed to join thread 1.");
    if (pthread_join(thread2, NULL))
        perror("Failed to join thread 2.");

    pthread_mutex_destroy(&mutex);

    return 0;
}
