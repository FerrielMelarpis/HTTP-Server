#include "httpserver.h"

int main(int argc, char **argv)
{
    char port[6]; // default
    char c;
    if((c = getopt(argc, argv, "p:")) != -1) {
        strncpy(port, optarg, 6);
        port[5] = '\0';
    }
    else
        strncpy(port, "8000\0", 5);
    HttpServer httpServer(port);
    httpServer.run();
    return 0;
}