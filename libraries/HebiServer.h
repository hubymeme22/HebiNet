#include "TCPService.h"
#include "CollEnc.h"

#include <cstring>
#include <string>
#include <map>

class HebiNetServer: public Server {
    private:
        const char* key;
        int keySize;

    public:
        HebiNetServer(const char*, int, const char*);

        virtual bool acceptCondition(char*, int);
        virtual void recieveProcess(int, char*, int);
};

// HebiNetServer constructor
HebiNetServer::HebiNetServer(const char* ip, int port, const char* key): Server(ip, port) {
    this->key = key;
    this->keySize = strlen(key);
}

// initial condition from packet to indicate if the client will be accepted
bool HebiNetServer::acceptCondition(char* buffer, int bufferSize) {
    return (strncmp(buffer, "hebi_child_server", 17) == 0);
}

// overriden recieve process from server
void HebiNetServer::recieveProcess(int id, char* buffer, int bufferSize) {
    if (this->acceptCondition(buffer, bufferSize))
        printf("Thread[%d] Recieved override: %s", id, buffer);

    // close the connection for the client if not accepted
    else this->closeConnection(id);
}
