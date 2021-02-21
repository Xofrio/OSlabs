#include <cstring>
#include <fcntl.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <sys/shm.h>
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
    int data      { 1 },
        sleepTime { *(&((Accessory*)information)->sleepTime) },
        *adress   { *(&((Accessory*)information)->adress) };
    bool *flag { &((Accessory*)information)->flag };
    sem_t *semaphoreWrite { *(&((Accessory*)information)->semaphoreWrite) },
          *semaphoreRead  { *(&((Accessory*)information)->semaphoreRead) };

    while(*flag) {
        if (data == 11)
            data = 1;
        
        printf("Generated value in writer lab6_1: %d\n", data);

        memcpy(adress, &data, sizeof(int));

        sem_post(semaphoreWrite);

        sem_wait(semaphoreRead);

        sleep(sleepTime);
        
        ++data;
    }
    return nullptr;
}

int main() {
    pthread_t thread;
    Accessory forThread { 1, true };
    int sharedMemory { shmget(ftok("/tmp", 'a'), sizeof(int), 0644 | IPC_CREAT) };

    forThread.adress = (int*)shmat(sharedMemory, nullptr, 0);

    forThread.semaphoreRead = sem_open("/semaphoreRead", O_CREAT, 0644, 0);
    forThread.semaphoreWrite = sem_open("/semaphoreWrite", O_CREAT, 0644, 0);

    if (pthread_create(&thread, nullptr, &thread_job, (void*)&forThread))
        perror("Failed to create thread in lab6_1.cpp.");

    getchar();

    forThread.flag = false;

    if (pthread_join(thread, nullptr))
        perror("Failed to join thread in lab6_1.cpp.");

    sem_close(forThread.semaphoreRead);
    sem_unlink("/semaphoreRead");
    
    sem_close(forThread.semaphoreWrite);
    sem_unlink("/semaphoreWrite");

    shmdt(nullptr);
    shmctl(sharedMemory, IPC_RMID, nullptr);

    return 0;
}
