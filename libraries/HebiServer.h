#include "TCPService.h"
#include "CollEnc.h"

#include <cstring>
#include <string>
#include <tuple>
#include <map>

using std::tuple;
using std::map;

class HebiNetServer: public Server {
    private:
        map<int, bool> acceptedClients;
        map<int, tuple<char*, int>> cliMsgHolder;

        const char* breaker;
        const char* key;
        int keySize;

        void appendMessage(int, char*, int);

    public:
        HebiNetServer(const char*, int, const char*);
        void setBreaker(const char*);
        void recieveProcess(int, char*, int);
        virtual void onRecieve(int, char*, int);
        virtual bool acceptCondition(char*, int);
};

// HebiNetServer constructor
HebiNetServer::HebiNetServer(const char* ip, int port, const char* key): Server(ip, port) {
    this->key = key;
    this->keySize = strlen(key);
    this->breaker = "&@&";
    setKey((char*)this->key, this->keySize);
}

// sets the breaker for packets, indicating the end of the packet
void HebiNetServer::setBreaker(const char* breaker) {
    if (strlen(breaker) < this->bufferLimit)
        this->breaker = breaker;
}

// initial condition from packet to indicate if the client will be accepted
bool HebiNetServer::acceptCondition(char* buffer, int bufferSize) {
    return (strncmp(buffer, this->key, this->keySize) == 0);
}

// overriden recieve process from server
void HebiNetServer::recieveProcess(int id, char* buffer, int bufferSize) {
    // for first time connection, confirm them by their initial packet
    if (acceptedClients.find(id) == acceptedClients.end()) {
        if (this->acceptCondition(buffer, bufferSize)) acceptedClients.insert(std::pair<int, bool>(id, true));
        else this->closeConnection(id);
        return;
    }

    bool breakerExists = true;
    int brLen = strlen(this->breaker);

    // checking the packet breaker at the end of the pointer
    for (int i = 0; i < brLen; i++)
        if (buffer[(bufferSize - 1) - brLen + i] != this->breaker[i])
            breakerExists = false;

    if (breakerExists) {
        if (cliMsgHolder.find(id) == cliMsgHolder.end()) {
            this->onRecieve(id, buffer, bufferSize);
        } else {
            // append the latest buffer and retrieve 
            this->appendMessage(id, buffer, bufferSize);
            tuple<char*, int> holderPtr = cliMsgHolder.at(id);

            char* buffstrd = std::get<0>(holderPtr);
            int buffsize = std::get<1>(holderPtr);
            this->onRecieve(id, buffstrd, buffsize);
        }
    }
}

// appends the message to the temporary message holder
void HebiNetServer::appendMessage(int id, char* buffer, int bufferSize) {
    if (cliMsgHolder.find(id) != cliMsgHolder.end()) {
        tuple<char*, int> holderPtr = cliMsgHolder.at(id);

        // check if there's a buffer assigned
        char* buffstrd = std::get<0>(holderPtr);
        int buffsize = std::get<1>(holderPtr);

        if ((buffstrd == NULL) || (buffstrd == 0)) {
            holderPtr = tuple<char*, int>(buffer, bufferSize);
            cliMsgHolder.emplace(std::pair<int, tuple<char*, int>>(id, holderPtr));
            return;
        }

        // append the data
        char* newDataHolder = new char[buffsize + bufferSize];
        for (int i = 0; i < (buffsize + bufferSize); i++) {
            if (i < bufferSize) newDataHolder[i] = buffstrd[i];
            else newDataHolder[i - buffsize] = buffer[i];
        }

        // update the client message information
        holderPtr = tuple<char*, int>(newDataHolder, (buffsize + bufferSize));
        cliMsgHolder.emplace(std::pair<int, tuple<char*, int>>(id, holderPtr));
    } else {
        tuple<char*, int> holderPtr = tuple<char*, int>(buffer, bufferSize);
        cliMsgHolder.insert(std::pair<int, tuple<char*, int>>(id, holderPtr));
    }
}

// the actual recieve of message (with complete)
void HebiNetServer::onRecieve(int, char*, int) {
}