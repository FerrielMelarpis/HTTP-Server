#ifndef HTTP_SERVER
#define HTTP_SERVER

#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <signal.h>
#include <fcntl.h>

#define BYTES 1024
#define MSGLEN 10000

class HttpServer {
public:
    HttpServer(char *);
    ~HttpServer();
    void run();
    void respond(int);
private:
    int listener;
    int clients[SOMAXCONN];
    char port[6];
    char *base;
};
#endif // HTTP_SERVER