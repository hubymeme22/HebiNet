#include "libraries/TCPService.h"

int main() {
    Client client = Client("0.0.0.0", 8080);
    client.connectByMain();

    char welcome[] = "&@&";
    client.sendMsg(welcome, 4);
    client.loopedListen();
}