#include <chrono>
#include <cstring>
#include <iostream>
#include <pthread.h>
#include <thread>

struct thread_parameters {
    bool flag;
    char data;
    size_t exit_code,
           seconds;

    explicit thread_parameters(char data, size_t exit_code, bool flag=true, size_t seconds=1)
    : data(data), exit_code(exit_code), flag(flag), seconds(seconds) {}
};

void *thread_routine(void *information) {
    bool *flag{ &reinterpret_cast<thread_parameters*>(information)->flag };
    const char symbol{ *&reinterpret_cast<thread_parameters*>(information)->data };
    const std::chrono::seconds sleep_time{ *&reinterpret_cast<thread_parameters*>(information)->seconds };

    while(*flag) {
        std::cout << symbol << std::flush;

        std::this_thread::sleep_for(sleep_time);
    }
    const size_t exit_code{ *&reinterpret_cast<thread_parameters*>(information)->exit_code };

    return reinterpret_cast<void*>(exit_code);
}

int main() {
    pthread_t thread1,
              thread2;
    thread_parameters for_thread1{ '1', 6 },
                      for_thread2{ '2', 9 };

    if (pthread_create(&thread1, nullptr, &thread_routine, reinterpret_cast<void*>(&for_thread1)))
        std::cerr << "Failed to create thread 1: " << strerror(errno) << '\n';

    if (pthread_create(&thread2, nullptr, &thread_routine, reinterpret_cast<void*>(&for_thread2)))
        std::cerr << "Failed to create thread 2: " << strerror(errno) << '\n';

    std::getchar();

    for_thread1.flag = for_thread2.flag = false;

    size_t result1{ 0 },
           result2{ 0 };

    if (pthread_join(thread1, reinterpret_cast<void**>(&result1)))
        std::cerr << "Failed to join thread 1: " << strerror(errno) << '\n';

    if (pthread_join(thread2, reinterpret_cast<void**>(&result2)))
        std::cerr << "Failed to join thread 2: " << strerror(errno) << '\n';

    std::cout << "Thread 1 return code: " << result1 << '\n';
    std::cout << "Thread 2 return code: " << result2 << '\n';

    return 0;
}
