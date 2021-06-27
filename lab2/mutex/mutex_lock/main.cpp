#include <chrono>
#include <cstring>
#include <iostream>
#include <pthread.h>
#include <thread>

struct thread_parameters {
    bool flag;
    char data;
    size_t number_of_symbols,
           seconds;
    static pthread_mutex_t mutex;

    explicit thread_parameters(char data, bool flag=true, size_t number_of_symbols=5, size_t seconds=1)
    : data(data), flag(flag), number_of_symbols(number_of_symbols), seconds(seconds) {}
};

void *thread_routine(void *information) {
    bool *flag{ &reinterpret_cast<thread_parameters*>(information)->flag };
    const char symbol{ *&reinterpret_cast<thread_parameters*>(information)->data };
    const size_t number_of_symbols{ *&reinterpret_cast<thread_parameters*>(information)->number_of_symbols };
    const std::chrono::seconds sleep_time{ *&reinterpret_cast<thread_parameters*>(information)->seconds };

    while(*flag) {
        pthread_mutex_lock(&thread_parameters::mutex);

        for (size_t i{ 0 }; i < number_of_symbols; ++i) {
            std::cout << symbol << std::flush;

            std::this_thread::sleep_for(sleep_time);
        }
        pthread_mutex_unlock(&thread_parameters::mutex);

        std::this_thread::sleep_for(sleep_time);
    }
    return nullptr;
}

pthread_mutex_t thread_parameters::mutex{ PTHREAD_MUTEX_INITIALIZER };

int main() {
    pthread_t thread1,
              thread2;
    thread_parameters for_thread1{ '1' },
                      for_thread2{ '2' };

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

    return 0;
}
