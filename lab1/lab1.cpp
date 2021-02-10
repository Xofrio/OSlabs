#include <pthread.h>
#include <stdio.h>
#include <unistd.h>

constexpr int sleepTime { 1 };

class Accessory {
    public:
        bool flag;
        char data;

        Accessory(char data, bool flag) {
            this->flag = flag;
            this->data = data;
        }
};

void *first_thread_job(void *information) {
    char symbol{ *(&((Accessory*)information)->data) };

    while(*(&((Accessory*)information)->flag)) {
        printf("%c", symbol);
        fflush(stdout);
        sleep(sleepTime);
    }
    pthread_exit((void*)223);
}

void *second_thread_job(void *information) {
    char symbol{ *(&((Accessory*)information)->data) };

    while(*(&((Accessory*)information)->flag)) {
        printf("%c", symbol);
        fflush(stdout);
        sleep(sleepTime);
    }
    pthread_exit((void*)322);
}

int main() {
    pthread_t thread1, thread2;
    Accessory forFirstThread    { '1', true },
              forSecondThread   { '2', true };

    pthread_create(&thread1, NULL, &first_thread_job, (void*)&forFirstThread);
    pthread_create(&thread2, NULL, &second_thread_job, (void*)&forSecondThread);

    getchar();

    forFirstThread.flag = forSecondThread.flag = false;
    int result1{},
        result2{};

    pthread_join(thread1, (void**)&result1);
    pthread_join(thread2, (void**)&result2);

    printf("Return code from thread one is: %d.\n", result1);
    printf("Return code from thread two is: %d.\n", result2);

    return 0;
}
