#include "TCPService.h"

class HebiNetServer: public Server {
    private:
        const char* key;
    public:
        HebiNetServer(const char*, int, const char*);
        void recieveProcess(char*, int);
        void loopedListen();
};

HebiNetServer::HebiNetServer(const char* ip, int port, const char* key): Server(ip, port) {
    this->key = key;
}

void HebiNetServer::recieveProcess(char* buffer, int bufferSize) {
    printf("Recieved override: %s", buffer);
}

void HebiNetServer::loopedListen() {
    Server::loopedListen();
}
