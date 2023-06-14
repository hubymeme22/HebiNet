#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <thread>
#include <tuple>
#include <map>

using std::map;
using std::tuple;
using std::thread;

#ifndef HEBISERVER_H
#define HEBISERVER_H

class Server {
    protected:
        const char* ip;
        int port, bufferLimit;
        bool listenLoop, listening, debug;

        // server details
        int serverFD;
        int addrLength;
        fd_set readfs;
        sockaddr_in addr;

        // client thread tracking
        map<int, tuple<bool*, thread*, int>> thTracker;

        // helper functions
        void resetCharBuff(char*, int);
        void loopedRecieve(int, int, bool*);

    public:
        Server(const char*, int);

        void setBufferLimit(int);
        void setDebug(bool);
        void start();
        void stop();

        void listenSingle();
        void loopedListen();
        void closeConnection(int);

        // functions bellow are to be overriden
        void listenProcess();
        void sendClient(int, char*, int);
        virtual void recieveProcess(int, char*, int);
};

class Client {
    protected:
        const char* ip;
        int port;

        struct sockaddr_in serverAddr;
        int clientFD;

        void resetCharBuff(char*, int);

    public:
        Client(const char*, int);

        void connectByMain();
        void loopedListen();
        void sendMsg(char*, int);
        virtual void recieveProcess(char*, int);
};


/////////////////////////
//  Server Definition  //
/////////////////////////
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
        this->recieveProcess(0, buffer, valRead);
    }
}

// listens in loop for handling multiple clients
void Server::loopedListen() {
    if (this->listening) {
        puts("[!] Server is already listening");
        return;
    }

    this->listening = true;
    int tmpIndex = 0;
    while (this->listenLoop) {
        int newSocket = accept(serverFD, (sockaddr*)&addr, (socklen_t*)&addrLength);
        if (this->debug) puts("[+] A new client has connected!");

        // spawn a new thread, and save its status
        bool* loopStatus = new bool;
        (*loopStatus) = true;


        // insert the threading and statuses to the tracker
        thread* th = new thread(&Server::loopedRecieve, this, newSocket, tmpIndex, loopStatus);
        thTracker.insert(std::pair<int, tuple<bool*, thread*, int>>(tmpIndex, tuple<bool*, thread*, int>(loopStatus, th, newSocket)));
        tmpIndex++;
    }
}

// for looped recieving of clients
void Server::loopedRecieve(int newSocket, int id, bool* indicator) {
    if (newSocket < 0) {
        perror("SingleClientAcceptError");
        exit(EXIT_FAILURE);
    }

    char* buffer = new char[bufferLimit];
    while (*indicator) {
        this->resetCharBuff(buffer, bufferLimit);
        int valRead = read(newSocket, buffer, bufferLimit);
        if (valRead <= 0) {
            if (this->debug) printf("[-] A client has disconnected (id=%d)\n", id);
            if (thTracker.find(id) != thTracker.end()) this->closeConnection(id);
            break;
        }

        // probably safe to use strlen since we are reseting
        // the buffer everytime when being called
        this->recieveProcess(id, buffer, valRead);
    }
}

// closes the connection by using the client id
void Server::closeConnection(int id) {
    tuple<bool*, thread*, int> pair = thTracker.at(id);

    // deallocate the addresses that are not needed anymore
    bool* inctr = std::get<0>(pair);
    thread* th = std::get<1>(pair);
    int socket = std::get<2>(pair);

    *inctr = false;
    close(socket);

    delete inctr, th;
    thTracker.erase(id);
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
void Server::recieveProcess(int id, char* buffer, int bufferSize) {
    printf("Thread[%d] Recieved: %s", id, buffer);
}

// can be overloaded soon
void Server::sendClient(int id, char* buffer, int bufferSize) {
    if (thTracker.find(id) != thTracker.end()) {
        tuple<bool*, thread*, int> info = thTracker.at(id);
        int socket = std::get<2>(info);

        send(socket, buffer, bufferSize, 0);
    }
}

/////////////////////////
//  Client definition  //
/////////////////////////
Client::Client(const char* ip, int port) {
    this->ip = ip;
    this->port = port;
}

// for resetting buffers
void Client::resetCharBuff(char* buffer, int bufferSize) {
    for (; bufferSize > 0; bufferSize--)
        buffer[bufferSize - 1] = 0;
}

// connects to the specified ip and port above
void Client::connectByMain() {
    if ((this->clientFD = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        puts("SocketError");
        exit(-1);
    }

    this->serverAddr.sin_family = AF_INET;
    this->serverAddr.sin_port = htons(this->port);

    if (inet_pton(AF_INET, this->ip, &this->serverAddr.sin_addr) <= 0) {
        puts("IPError");
        exit(-1);
    }

    int status;
    if ((status = connect(this->clientFD, (struct sockaddr*)&this->serverAddr, sizeof(this->serverAddr))) < 0) {
        puts("ServerConnectionError");
        exit(-1);
    }
}

// listens all the incoming messages through loop
void Client::loopedListen() {
    int messageSize;
    char* buffer = new char[5024];

    while (true) {
        this->resetCharBuff(buffer, 5024);
        messageSize = read(this->clientFD, buffer, sizeof(char)*5024);

        if (messageSize == 0) break;
        this->recieveProcess(buffer, messageSize);
    }
}

// sends a message buffer
void Client::sendMsg(char* buffer, int bufferSize) {
    send(this->clientFD, buffer, (size_t)bufferSize, 0);
}

// when client has recieved a message
void Client::recieveProcess(char* buffer, int bufferSize) {
    printf("the message: %s\n", buffer);
}

#endif