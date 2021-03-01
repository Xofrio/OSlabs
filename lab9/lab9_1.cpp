#include <arpa/inet.h>
#include <cstring>
#include <fcntl.h>
#include <netinet/in.h>
#include <pthread.h>
#include <queue>
#include <stdio.h>
#include <string>
#include <sys/signal.h>
#include <sys/socket.h>
#include <sys/syscall.h>
#include <sys/utsname.h>
#include <unistd.h>

constexpr size_t bufferSize { 256 },
                 sleepTime  { 1 };

class Accessory {
    public:
        int listenSocket;
        bool receiverFlag,
             handlerFlag;
        std::vector <std::string> messageList;
        pthread_t receiverThread,
                  handlerThread;
        sockaddr_in listenSocketAddress,
                    serverSocketAddress;
        socklen_t addressLength;
        
        static pthread_mutex_t mutex;

        Accessory(bool receiverFlag, bool handlerFlag) {
            this->receiverFlag = receiverFlag;
            this->handlerFlag = handlerFlag;
        }
};

void *receiver(void *information) {
    bool *flag { &((Accessory*)information)->receiverFlag };
    char buffer[bufferSize];

    while (*flag) {
        memset(buffer, 0, bufferSize);
        
        sleep(2 * sleepTime);

        ssize_t receiveStatus = recvfrom(*(&((Accessory*)information)->listenSocket), (void*)buffer, bufferSize, 0, (sockaddr*)&((Accessory*)information)->serverSocketAddress, &((Accessory*)information)->addressLength);

        if (receiveStatus == -1) {
            perror("Couldn't receive the request. Error");

            sleep(sleepTime);
        }
        else if (receiveStatus == 0) {
            printf("Shutdown performed. You should terminate this program with Enter.\n");

            sleep(sleepTime);
        }
        else {
            pthread_mutex_lock(&Accessory::mutex);

            (&((Accessory*)information)->messageList)->push_back((std::string)buffer);

            pthread_mutex_unlock(&Accessory::mutex);
        }
    }
    pthread_exit(nullptr);
}

void *handler(void *information) {
    int count { 0 };
    bool *flag { &((Accessory*)information)->handlerFlag };

    while (*flag) {
        pthread_mutex_lock(&Accessory::mutex);

        if (!(&((Accessory*)information)->messageList)->empty()) {
            std::string request { (&((Accessory*)information)->messageList)->back() };

            (&((Accessory*)information)->messageList)->pop_back();

            printf("New request: %s\n", request.c_str());

            pthread_mutex_unlock(&Accessory::mutex);

            utsname systemInformation;

            uname(&systemInformation);

            char buffer[bufferSize] { '\0' };

            if (!count)
                snprintf(buffer, bufferSize, "Domain name:\t%s", systemInformation.domainname);
            else if (count == 1)
                snprintf(buffer, bufferSize, "Machine name:\t%s", systemInformation.machine);
            else if (count == 2)
                snprintf(buffer, bufferSize, "Node name:\t%s", systemInformation.nodename);
            else if (count == 3)
                snprintf(buffer, bufferSize, "Release:\t%s", systemInformation.release);
            else if (count == 4)
                snprintf(buffer, bufferSize, "System name:\t%s", systemInformation.sysname);
            else if (count == 5)
                snprintf(buffer, bufferSize, "Version:\t%s", systemInformation.version);

            ssize_t sendStatus { sendto(*(&((Accessory*)information)->listenSocket), (void*)buffer, bufferSize, 0, (sockaddr*)&((Accessory*)information)->serverSocketAddress, *(&((Accessory*)information)->addressLength)) };

            if (sendStatus == -1)
                perror("Couldn't send the message. Error");
            else {
                printf("The request has been processed & the message has been sent to the client!\n\n");
                ++count;
                if (count == 6)
                    count = 0;
            }
        }
        else {
            pthread_mutex_unlock(&Accessory::mutex);

            sleep(sleepTime);
        }
    }
    pthread_exit(nullptr);
}

pthread_mutex_t Accessory::mutex ( PTHREAD_MUTEX_INITIALIZER );

int main() {
    Accessory forThreads { true, true };

    forThreads.listenSocket = socket(AF_INET, SOCK_DGRAM, 0);

    fcntl(forThreads.listenSocket, F_SETFL, O_NONBLOCK);

    forThreads.listenSocketAddress.sin_family = AF_INET;
    forThreads.listenSocketAddress.sin_port = htons(8000);
    forThreads.listenSocketAddress.sin_addr.s_addr = htonl(INADDR_ANY);

    bind(forThreads.listenSocket, (sockaddr *)&forThreads.listenSocketAddress, sizeof(forThreads.listenSocketAddress));

    int optionValue { 1 };

    setsockopt(forThreads.listenSocket, SOL_SOCKET, SO_REUSEADDR, &optionValue, sizeof(optionValue));

    memset(&forThreads.serverSocketAddress, 0, sizeof(forThreads.serverSocketAddress));

    forThreads.serverSocketAddress.sin_family = AF_INET;
    forThreads.serverSocketAddress.sin_port = htons(7000);
    forThreads.serverSocketAddress.sin_addr.s_addr = inet_addr("127.0.0.1");

    forThreads.addressLength = sizeof(forThreads.serverSocketAddress);

    if (pthread_create(&forThreads.receiverThread, nullptr, &receiver, (void*)&forThreads))
        perror("Failed to create receiver thread in lab9_1. Error");
    if (pthread_create(&forThreads.handlerThread, nullptr, &handler, (void*)&forThreads))
        perror("Failed to create handler thread in lab9_1. Error");

    getchar();
    
    forThreads.receiverFlag = forThreads.handlerFlag = false;

    if(pthread_join(forThreads.receiverThread, nullptr))
        perror("Failed to join receiver thread in lab9_1. Error");
    if(pthread_join(forThreads.handlerThread, nullptr))
        perror("Failed to join handler thread in lab9_1. Error");
    
    close(forThreads.listenSocket);

    return 0;
}
