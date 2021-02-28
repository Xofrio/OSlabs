#include <cstring>
#include <fcntl.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <sys/mman.h>
#include <unistd.h>

class Accessory {
    public:
        int sleepTime,
            *adress;
        bool flag;
        sem_t *semaphoreRead,
              *semaphoreWrite;

        Accessory(int sleepTime, bool flag) {
            this->sleepTime = sleepTime;
            this->flag = flag;
        }
};

void *thread_job(void *information) {
    int data      { 0 },
        sleepTime { *(&((Accessory*)information)->sleepTime) },
        *adress   { *(&((Accessory*)information)->adress) };
    bool *flag { &((Accessory*)information)->flag };
    sem_t *semaphoreWrite { *(&((Accessory*)information)->semaphoreWrite) },
          *semaphoreRead  { *(&((Accessory*)information)->semaphoreRead) };

    while(*flag) {
        sem_wait(semaphoreWrite);

        memcpy(&data, adress, sizeof(int));

        printf("Value from writer lab6_11: %d\n", data);

        sem_post(semaphoreRead);

        sleep(sleepTime);
    }
    return nullptr;
}

int main() {
    pthread_t thread;
    Accessory forThread { 1, true };
    int sharedMemory { shm_open("/sharedMemory", O_CREAT | O_RDWR, 0644) };

    ftruncate(sharedMemory, sizeof(int));

    forThread.adress = (int*)mmap(0,sizeof(int), PROT_WRITE | PROT_READ, MAP_SHARED, sharedMemory, 0);

    forThread.semaphoreRead = sem_open("/semaphoreRead", O_CREAT, 0644, 0);
    forThread.semaphoreWrite = sem_open("/semaphoreWrite", O_CREAT, 0644, 0);

    if (pthread_create(&thread, nullptr, &thread_job, (void*)&forThread))
        perror("Failed to create thread in lab6_21. Error");

    getchar();

    forThread.flag = false;

    if (pthread_join(thread, nullptr))
        perror("Failed to join thread in lab6_21. Error");

    sem_close(forThread.semaphoreRead);
    sem_unlink("/semaphoreRead");
    
    sem_close(forThread.semaphoreWrite);
    sem_unlink("/semaphoreWrite");

    munmap(forThread.adress, sizeof(int));
    close(sharedMemory);
    shm_unlink("/sharedMemory");

    return 0;
}
