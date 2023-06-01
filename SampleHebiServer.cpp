#include "libraries/HebiServer.h"

int main() {
    HebiNetServer* server = new HebiNetServer("0.0.0.0", 8080, "rickandmorty");
    server->setBufferLimit(520);
    server->setDebug(true);
    server->start();
    server->loopedListen();
}