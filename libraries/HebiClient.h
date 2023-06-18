#ifndef HEBICLIENT_H
#define HEBICLIENT_H
#include "TCPService.h"
#include "CollEnc.h"
#include <cstring>

class HebiNetClient: public Client {
    protected:
        char* messageHolder;
        int messageSize;

        const char* breaker;
        const char* key;
        int keySize;

        char* appendBreaker(char*, int);

    public:
        HebiNetClient(const char*, int, const char*);

        void connectByMain();
        void recieveProcess(char*, int);
        void setBreaker(const char*);
        virtual void sendMsg(char*, int);
        virtual void onRecieve(char*, int);
};

/*
    Initializes the IP and port to which server to be connected on
    key will be used for encryption and decryption of incoming packets
*/
HebiNetClient::HebiNetClient(const char* ip, int port, const char* key): Client(ip, port) {
    this->key = key;
    this->keySize = strlen(key);
    this->breaker = "&@&";
    setKey((char*)key, this->keySize);
}

// appends the breaker string to the end of message
char* HebiNetClient::appendBreaker(char* buffer, int bufferSize) {
    int breakerSize = strlen(this->breaker);
    char* newBuffer = new char[bufferSize + breakerSize + 1];

    for (int i = 0; i < (bufferSize + breakerSize); i++) {
        if (i < bufferSize) {
            newBuffer[i] = buffer[i];
            continue;
        }
        newBuffer[i] = this->breaker[i - bufferSize];
    }

    // append terminating character
    newBuffer[bufferSize + breakerSize] = 0;
    return newBuffer;
}

// redefine connection by sending a breaker first (to properly connect to the server)
void HebiNetClient::connectByMain() {
    Client::connectByMain();

    int brLen = strlen(this->breaker);
    Client::sendMsg(bufferEncrypt((char*)this->breaker, brLen), brLen);
}

// decrypts the packets and parse the message (will be applied soon)
void HebiNetClient::recieveProcess(char* buffer, int bufferSize) {
    buffer = bufferDecrypt(buffer, bufferSize);
    this->onRecieve(buffer, bufferSize);
}

// sets a new packet breaker
void HebiNetClient::setBreaker(const char* breaker) {
    this->breaker = breaker;
}

void HebiNetClient::sendMsg(char* buffer, int bufferSize) {
    buffer = this->appendBreaker(buffer, bufferSize);
    int newBuffSize = bufferSize + strlen(this->breaker) + 1;

    buffer = bufferEncrypt(buffer, newBuffSize);
    Client::sendMsg(buffer, newBuffSize);
}

// process whenever a packet is recieved
void HebiNetClient::onRecieve(char* buffer, int bufferSize) {
}

#endif