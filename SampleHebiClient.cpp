#include "libraries/HebiClient.h"

int main() {
    HebiNetClient client = HebiNetClient("0.0.0.0", 8080, "rickandmorty");
    client.connectByMain();
    client.loopedListen();
}