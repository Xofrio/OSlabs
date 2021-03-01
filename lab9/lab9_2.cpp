#include <arpa/inet.h>
#include <cstring>
#include <fcntl.h>
#include <netinet/in.h>
#include <pthread.h>
#include <queue>
#include <signal.h>
#include <stdio.h>
#include <sys/signal.h>
#include <sys/socket.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <sys/utsname.h>
#include <unistd.h>

constexpr size_t bufferSize { 256 },
                 sleepTime  { 1 };

class Accessory {
    public:
        int clientSocket;
        bool receiverFlag,
             requesterFlag;
        pthread_t receiverThread,
                  requesterThread;
        sockaddr_in clientSocketAddress;
        socklen_t addressLength;
        
        Accessory(bool receiverFlag, bool requesterFlag) {
            this->receiverFlag = receiverFlag;
            this->requesterFlag = requesterFlag;
        }
};

void signal_handler(int signalNumber) {
    printf("I've managed to intercept SIGPIPE signal.\n"
           "Press Enter to terminate this program with a 5 second delay.\n");
    
    sleep(5 * sleepTime);
}

void *requester(void *information) {
    int clientSocket  { *(&((Accessory*)information)->clientSocket) },
        requestsCount { 0 };
    bool *flag { &((Accessory*)information)->requesterFlag };
    char buffer[bufferSize] { '\0' };
    
    while(*flag) {
        int length { snprintf(buffer, bufferSize, "Request #%d", requestsCount) };

        sleep(sleepTime);

        ssize_t sendStatus { sendto(clientSocket, &buffer, (size_t)length, 0, (sockaddr*)&((Accessory*)information)->clientSocketAddress, sizeof(*(&((Accessory*)information)->clientSocketAddress))) };

        if (sendStatus == -1) {
            perror("Couldn't send the request. Error");

            sleep(sleepTime);
        }
        ++requestsCount;

        sleep(sleepTime);
    }
    pthread_exit(nullptr);
}

void *receiver(void *information) {
    int clientSocket { *(&((Accessory*)information)->clientSocket) };
    bool *flag { &((Accessory*)information)->receiverFlag };
    char buffer[bufferSize] { '\0' };

    while(*flag) {
        memset(buffer, 0, bufferSize);

        sleep(2 * sleepTime);

        ssize_t receiveStatus { recvfrom(clientSocket, (void*)&buffer, bufferSize, 0, (sockaddr*)&((Accessory*)information)->clientSocketAddress, &((Accessory*)information)->addressLength) };

        if (receiveStatus == -1) {
            perror("Couldn't receive message. Error");

            sleep(sleepTime);
        }
        else if (receiveStatus > 0)
            printf("\nClient got message:\n%s\n", buffer);
        
        sleep(sleepTime);
    }
    pthread_exit(nullptr);
}

int main() {
    Accessory forThreads { true, true };

    signal(SIGPIPE, &signal_handler);

    forThreads.clientSocket = socket(AF_INET, SOCK_DGRAM, 0);

    fcntl(forThreads.clientSocket, F_SETFL, O_NONBLOCK);

    forThreads.clientSocketAddress.sin_family = AF_INET;
    forThreads.clientSocketAddress.sin_port = htons(8000);
    forThreads.clientSocketAddress.sin_addr.s_addr = inet_addr("127.0.0.1");

    forThreads.addressLength = sizeof(forThreads.clientSocketAddress);

    if (pthread_create(&forThreads.receiverThread, nullptr, &receiver, (void*)&forThreads))
        perror("Failed to create receiver thread in lab9_2. Error");
    if (pthread_create(&forThreads.requesterThread, nullptr, &requester, (void*)&forThreads))
        perror("Failed to create requester thread in lab9_2. Error");
    
    getchar();

    forThreads.receiverFlag = forThreads.requesterFlag = false;

    if (pthread_join(forThreads.receiverThread, nullptr))
        perror("Failed to join receiver thread in lab9_2. Error");
    if (pthread_join(forThreads.requesterThread, nullptr))
        perror("Failed to join requester thread in lab9_2. Error");
    
    close(forThreads.clientSocket);

    return 0;
}