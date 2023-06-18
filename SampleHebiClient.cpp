#include "libraries/HebiClient.h"
#include <iostream>

using std::string;
using std::cin;
using std::cout;
using std::getline;

int main() {
    HebiNetClient client = HebiNetClient("0.0.0.0", 8080, "rickandmorty");
    client.connectByMain();
    client.threadedListen();

    while (true) {
        string message;
        cout << "Enter a message: ";
        getline(cin, message);

        const char* msg = message.c_str();
        client.sendMsg((char*)msg, strlen(msg));
    }
}