#ifndef HEBICLIENT_H
#define HEBICLIENT_H
#include "TCPService.h"
#include <cstring>

class HebiNetClient: public Client {
    protected:
        char* messageHolder;
        int messageSize;

        const char* breaker;
        const char* key;
        int keySize;

        void appendMessage(char*, int);

    public:
        HebiNetClient(const char*, int, const char*);

        void connectByMain();
        // void sendMsg(char*, int);
        void recieveProcess(char*, int);
        void setBreaker(const char*);
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
}

// redefine connection by sending a breaker first (to properly connect to the server)
void HebiNetClient::connectByMain() {
    Client::connectByMain();

    // convert the breaker into char*
    char* convertedBreaker = new char[strlen(this->breaker)];
    for (int i = 0; i < strlen(breaker); i++)
        convertedBreaker[i] = this->breaker[i];

    Client::sendMsg(convertedBreaker, strlen(this->breaker));
}

// decrypts the packets and parse the message (will be applied soon)
void HebiNetClient::recieveProcess(char* buffer, int bufferSize) {
    this->onRecieve(buffer, bufferSize);
}

// sets a new packet breaker
void HebiNetClient::setBreaker(const char* breaker) {
    this->breaker = breaker;
}

// process whenever a packet is recieved
void HebiNetClient::onRecieve(char* buffer, int bufferSize) {
}

#endif