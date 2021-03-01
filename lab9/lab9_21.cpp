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
        bool connectorFlag,
             receiverFlag,
             requesterFlag;
        pthread_t connectorThread,
                  receiverThread,
                  requesterThread;
        sockaddr_in clientSocketAddress;
        
        Accessory(bool connectorFlag, bool receiverFlag, bool requesterFlag) {
            this->connectorFlag = connectorFlag;
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
        int length { snprintf(buffer, sizeof(buffer), "Request #%d", requestsCount) };
        ssize_t sendStatus { send(clientSocket, (void*)&buffer, (size_t)length, 0) };

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

        ssize_t receiveStatus { recv(clientSocket, (void*)&buffer, bufferSize, 0) };

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

void *connector(void *information) {
    int clientSocket { *(&((Accessory*)information)->clientSocket) };
    bool *flag { &((Accessory*)information)->connectorFlag };

    while(*flag) {
        int result { connect(clientSocket, (sockaddr*)&((Accessory*)information)->clientSocketAddress, sizeof(((Accessory*)information)->clientSocketAddress)) };

        if (result == -1) { 
            perror("Couldn't establish a connection. Error");
            
            sleep(sleepTime);
        }
        else {
            printf("Connection established!\n\n");

            if (pthread_create(&((Accessory*)information)->requesterThread, nullptr, &requester, information))
                perror("Failed to create requester thread in lab9_21. Error");
            if (pthread_create(&((Accessory*)information)->receiverThread, nullptr, &receiver, information))
                perror("Failed to create receiver thread in lab9_21. Error");
            
            pthread_exit(nullptr);
        }
    }
    pthread_exit(nullptr);
}

int main() {
    Accessory forThreads { true, true, true };

    signal(SIGPIPE, &signal_handler);

    forThreads.clientSocket = socket(AF_INET, SOCK_STREAM, 0);

    fcntl(forThreads.clientSocket, F_SETFL, O_NONBLOCK);

    forThreads.clientSocketAddress.sin_family = AF_INET;
    forThreads.clientSocketAddress.sin_port = htons(7000);
    forThreads.clientSocketAddress.sin_addr.s_addr = inet_addr("127.0.0.1");

    if (pthread_create(&forThreads.connectorThread, nullptr, &connector, (void*)&forThreads))
        perror("Failed to create connector thread in lab9_21. Error");

    getchar();

    forThreads.connectorFlag = forThreads.receiverFlag = forThreads.requesterFlag = false;

    if (pthread_join(forThreads.connectorThread, nullptr))
        perror("Failed to join connector thread in lab9_21. Error");
    if (pthread_join(forThreads.receiverThread, nullptr))
        perror("Failed to join receiver thread in lab9_21. Error");
    if (pthread_join(forThreads.requesterThread, nullptr))
        perror("Failed to join requester thread in lab9_21. Error");

    shutdown(forThreads.clientSocket, 2);
    
    close(forThreads.clientSocket);

    return 0;
}