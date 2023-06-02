#include "TCPService.h"
#include "CollEnc.h"

#include <cstring>
#include <string>
#include <map>

using std::map;

class HebiNetServer: public Server {
    private:
        map<int, bool> acceptedClients;
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
    return (strncmp(buffer, this->key, this->keySize) == 0);
}

// overriden recieve process from server
void HebiNetServer::recieveProcess(int id, char* buffer, int bufferSize) {
    // for first time connection, confirm them by their initial packet
    if (acceptedClients.find(id) == acceptedClients.end()) {
        if (this->acceptCondition(buffer, bufferSize))
            acceptedClients.insert(std::pair<int, bool>(id, true));
        else
            this->closeConnection(id);
    } else {
        printf("Thread[%d] says: %s", id, buffer);
    }
}
