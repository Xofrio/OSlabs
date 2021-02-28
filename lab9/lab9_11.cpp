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
        int listenSocket,
            serverSocket;
        bool connectorFlag,
             receiverFlag,
             handlerFlag;
        std::vector <std::string> messageList;
        pthread_t connectorThread,
                  receiverThread,
                  handlerThread;
        sockaddr_in listenSocketAddress,
                    serverSocketAddress;
        
        static pthread_mutex_t mutex;

        Accessory(bool connectorFlag, bool receiverFlag, bool handlerFlag) {
            this->connectorFlag = connectorFlag;
            this->receiverFlag = receiverFlag;
            this->handlerFlag = handlerFlag;
        }
};

void *receiver(void *information) {
    bool *flag { &((Accessory*)information)->receiverFlag };
    char buffer[bufferSize];

    while (*flag) {
        int receiveStatus = recv(*(&((Accessory*)information)->serverSocket), (void*)buffer, bufferSize, 0);

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

            char answer[bufferSize] { '\0' };

            if (!count)
                snprintf(answer, sizeof(answer), "Domain name:\t%s", systemInformation.domainname);
            else if (count == 1)
                snprintf(answer, sizeof(answer), "Machine name:\t%s", systemInformation.machine);
            else if (count == 2)
                snprintf(answer, sizeof(answer), "Node name:\t%s", systemInformation.nodename);
            else if (count == 3)
                snprintf(answer, sizeof(answer), "Release:\t%s", systemInformation.release);
            else if (count == 4)
                snprintf(answer, sizeof(answer), "System name:\t%s", systemInformation.sysname);
            else if (count == 5)
                snprintf(answer, sizeof(answer), "Version:\t%s", systemInformation.version);

            ssize_t sendStatus { send(*(&((Accessory*)information)->serverSocket), (void*)answer, sizeof(answer), 0) };

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

void *connector(void *information) {
    bool *flag { &((Accessory*)information)->connectorFlag };
    socklen_t addressLength = (socklen_t)sizeof(*(&((Accessory*)information)->serverSocketAddress));

    while(*flag) {
        *(&((Accessory*)information)->serverSocket) = accept(*(&((Accessory*)information)->listenSocket), (sockaddr *)&((Accessory*)information)->serverSocketAddress, &addressLength);

        if (*(&((Accessory*)information)->serverSocket) == -1) {
            perror("Couldn't establish a connection. Error");
            
            sleep(sleepTime);
        }
        else {
            printf("Connection established!\n\n");
            if (pthread_create(&((Accessory*)information)->handlerThread, nullptr, &handler, information))
                perror("Failed to create handler thread in lab9_11. Error");
            if (pthread_create(&((Accessory*)information)->receiverThread, nullptr, &receiver, information))
                perror("Failed to create receiver thread in lab9_11. Error");

            pthread_exit(nullptr);
        }
    }
    pthread_exit(nullptr);
}

pthread_mutex_t Accessory::mutex ( PTHREAD_MUTEX_INITIALIZER );

int main() {
    Accessory forThreads { true, true, true };

    forThreads.listenSocket = socket(AF_INET, SOCK_STREAM, 0);

    fcntl(forThreads.listenSocket, F_SETFL, O_NONBLOCK);

    forThreads.listenSocketAddress.sin_family = AF_INET;
    forThreads.listenSocketAddress.sin_port = htons(7000);
    forThreads.listenSocketAddress.sin_addr.s_addr = htonl(INADDR_ANY);

    bind(forThreads.listenSocket, (sockaddr *)&forThreads.listenSocketAddress, sizeof(forThreads.listenSocketAddress));

    int optionValue { 1 };

    setsockopt(forThreads.listenSocket, SOL_SOCKET, SO_REUSEADDR, &optionValue, sizeof(optionValue));

    listen(forThreads.listenSocket,SOMAXCONN);

    if (pthread_create(&forThreads.connectorThread, nullptr, &connector, (void*)&forThreads))
        perror("Failed to create connector thread in lab9_11. Error");

    getchar();

    forThreads.connectorFlag = forThreads.receiverFlag = forThreads.handlerFlag = false;

    if(pthread_join(forThreads.connectorThread, nullptr))
        perror("Failed to join connector thread in lab9_11. Error");
    if(pthread_join(forThreads.receiverThread, nullptr))
        perror("Failed to join receiver thread in lab9_11. Error");
    if(pthread_join(forThreads.handlerThread, nullptr))
        perror("Failed to join handler thread in lab9_11. Error");

    shutdown(forThreads.serverSocket, 2);
    
    close(forThreads.listenSocket);
    close(forThreads.serverSocket);

    return 0;
}
