#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

class Server {
    private:
        const char* ip;

        int port;
        int bufferLimit;

        bool listenLoop;
        bool listening;
        bool debug;

        // server details
        int serverFD;
        int addrLength;
        fd_set readfs;
        sockaddr_in addr;

        // helper functions
        void resetCharBuff(char*, int);

    public:
        Server(const char*, int);

        void setBufferLimit(int);
        void setDebug(bool);
        void start();
        void stop();

        void listenSingle();
        void loopedListen();

        // functions bellow are to be overriden
        void listenProcess();
        void recieveProcess(char*, int);
        void sendClient(char*);
};

// initializes the ip and port to be used
Server::Server(const char* ip, int port) {
    this->ip = ip;
    this->port = port;
    this->bufferLimit = 1024;
    this->listenLoop = true;
    this->debug = false;
    this->listening = false;
}

// sets the limit of buffer that will be recieved from the client
void Server::setBufferLimit(int bufferLimit) {
    this->bufferLimit = bufferLimit;
}

/*
    - starts the server by binding the ip and port
    - loop and listens to the incoming clients
    - listens for incoming connection.
    - to change the rules for accepting clientsplease override the function "void listenProcess()"
*/
void Server::start() {
    addrLength = sizeof(addr);
    int opt=1;

    if (this->debug) {
        puts("[*] Starting server...");
        puts("[*] Creating socket...");
    }

    // for checking if the socket is successfully created
    if ((serverFD = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("SocketCreationFailed");
        exit(EXIT_FAILURE);
    }

    if (this->debug) {
        puts("[+] Successfully created socket!");
        puts("[*] Binding IP and PORT...");
    }

    // setting socket option
    if (setsockopt(serverFD, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
        perror("SockOptFailure");
        exit(EXIT_FAILURE);
    }

    // setting sockaddr_in properties
    addr.sin_family = AF_INET;
    addr.sin_port = htons(this->port);
    addr.sin_addr.s_addr = inet_addr(ip);

    // binding ip and port
    if (bind(serverFD, (sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("AddrPortBindFailure");
        exit(EXIT_FAILURE);
    }

    if (this->debug) {
        puts("[+] Successfully binded ip and port!");
        puts("[*] Listening for incoming clients");
    }

    if (listen(serverFD, 5) < 0) {
        perror("ListeningError");
        exit(EXIT_FAILURE);
    }
}

// listens for single client connection
void Server::listenSingle() {
    int newSocket;
    if ((newSocket = accept(serverFD, (sockaddr*)&addr, (socklen_t*)&addrLength)) < 0) {
        perror("SingleClientAcceptError");
        exit(EXIT_FAILURE);
    }

    char* buffer = new char[bufferLimit];
    while (true) {
        this->resetCharBuff(buffer, bufferLimit);
        int valRead = read(newSocket, buffer, bufferLimit);
        if (valRead <= 0) break;

        // probably safe to use strlen since we are reseting
        // the buffer everytime when being called
        this->recieveProcess(buffer, strlen(buffer));
    }
}

// for resetting buffers
void Server::resetCharBuff(char* buffer, int bufferSize) {
    for (; bufferSize > 0; bufferSize--)
        buffer[bufferSize - 1] = 0;
}

// for stopping the loop (also for threading purposes)
void Server::stop() { this->listenLoop = false; }

// for printing server status
void Server::setDebug(bool debugStatus) { this->debug = debugStatus; }

// to be overloaded soon
void Server::recieveProcess(char* buffer, int bufferSize) {
    printf("Recieved: %s", buffer);
}