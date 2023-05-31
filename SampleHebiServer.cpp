#include "libraries/TCPService.h"

int main() {
    Server* server = new Server("0.0.0.0", 8080);
    server->setBufferLimit(520);
    server->setDebug(true);
    server->start();
    server->listenSingle();
}