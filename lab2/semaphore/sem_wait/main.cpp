#include <chrono>
#include <cstring>
#include <iostream>
#include <pthread.h>
#include <semaphore.h>
#include <thread>

struct thread_parameters {
    bool flag;
    char data;
    size_t number_of_symbols,
           seconds;

    explicit thread_parameters(char data, bool flag=true, size_t number_of_symbols=5, size_t seconds=1)
    : data(data), flag(flag), number_of_symbols(number_of_symbols), seconds(seconds) {}
};

sem_t semaphore;

void *thread_routine(void *information) {
    bool *flag{ &reinterpret_cast<thread_parameters*>(information)->flag };
    const char symbol{ *&reinterpret_cast<thread_parameters*>(information)->data };
    const size_t number_of_symbols{ *&reinterpret_cast<thread_parameters*>(information)->number_of_symbols };
    const std::chrono::seconds sleep_time{ *&reinterpret_cast<thread_parameters*>(information)->seconds };

    while(*flag) {
        sem_wait(&semaphore);

        for (size_t i{ 0 }; i < number_of_symbols; ++i) {
            std::cout << symbol << std::flush;

            std::this_thread::sleep_for(sleep_time);
        }
        sem_post(&semaphore);

        std::this_thread::sleep_for(sleep_time);
    }
    return nullptr;
}

int main() {
    pthread_t thread1,
              thread2;
    thread_parameters for_thread1{ '1' },
                      for_thread2{ '2' };

    sem_init(&semaphore, 0, 1);

    if (pthread_create(&thread1, nullptr, &thread_routine, reinterpret_cast<void*>(&for_thread1)))
        std::cerr << "Failed to create thread 1: " << strerror(errno) << '\n';

    if (pthread_create(&thread2, nullptr, &thread_routine, reinterpret_cast<void*>(&for_thread2)))
        std::cerr << "Failed to create thread 2: " << strerror(errno) << '\n';

    std::getchar();

    for_thread1.flag = for_thread2.flag = false;

    if (pthread_join(thread1, nullptr))
        std::cerr << "Failed to join thread 1: " << strerror(errno) << '\n';

    if (pthread_join(thread2, nullptr))
        std::cerr << "Failed to join thread 2: " << strerror(errno) << '\n';

    sem_destroy(&semaphore);

    return 0;
}
