#include "httpserver.h"

HttpServer::HttpServer(char *port)
{
    strncpy(this->port, port, 6);
    this->base = getenv("PWD");
    for(int i = 0; i < SOMAXCONN; i++)
        this->clients[i] = -1;
}

HttpServer::~HttpServer()
{
}

void HttpServer::run()
{
    struct addrinfo hints, *res, *p;
    struct sockaddr_in clientaddr;
    socklen_t addrlen;
    // getaddrinfo for host
    memset (&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    if(getaddrinfo(NULL, this->port, &hints, &res) != 0) {
        perror ("getaddrinfo() error");
        exit(1);
    }
    // create socket and bind
    for(p = res; p!=NULL; p=p->ai_next) {
        this->listener = socket(p->ai_family, p->ai_socktype, 0);
        if(this->listener == -1) 
            continue;
        if(bind(this->listener, p->ai_addr, p->ai_addrlen) == 0)
            break;
    }
    if(p == NULL) {
        perror ("socket() or bind()");
        exit(1);
    }
    freeaddrinfo(res);
    // listen for incoming connections
    if(listen (this->listener, SOMAXCONN) != 0) {
        perror("listen() error");
        exit(1);
    }
    printf("\x1b[33mHTTP Server started at port:\x1b[0m\x1b[36m%s\x1b[0m\n", this->port);
    int id = 0;
    while(true) {
        addrlen = sizeof(clientaddr);
        clients[id] = accept(this->listener, (struct sockaddr *) &clientaddr, &addrlen);
        if (clients[id] < 0) {
            perror("accept() error");
            exit(-1);
        }
        else {
            if(fork() == 0) {
                respond(id);
                exit(0);
            }
        }
        while(clients[id] != -1) id = (id + 1) % SOMAXCONN;
    }
}

void HttpServer::respond(int id)
{
    char msg[MSGLEN], *request[3], data_to_send[BYTES], path[MSGLEN];
    int rcverr, fd, bytes_read;
    memset((void*) msg, (int) '\0', MSGLEN);
    rcverr=recv(clients[id], msg, MSGLEN, 0);
    if(rcverr < 0) // receive error
        perror("recv() error\n");
    else if(rcverr == 0) // receive socket closed
        fprintf(stderr, "Client has disconnected.\n");
    else  { // message received
        printf("\x1b[33m%s\n\x1b[0m", msg);
        request[0] = strtok(msg, " \t\n");
        if(strncmp(request[0], "GET\0", 4) == 0) {
            request[1] = strtok(NULL, " \t");
            request[2] = strtok(NULL, " \t\n");
            if(strncmp(request[2], "HTTP/1.0", 8) != 0 
                && strncmp(request[2], "HTTP/1.1", 8) != 0)
                write(this->clients[id], "HTTP/1.0 400 Bad Request\n", 25);
            else {
                // append index.html if requesting for root

                strcpy(path, this->base);
                if(strchr(request[1], '?') != NULL) {
                    int counter = 1;
                    char *end_params;
                    char *params = strtok_r(request[1], "?", &end_params);
                    FILE *index = fopen("index.html", "w");
                    if(strncmp(params, "/", 2) == 0)
                        strcpy(&path[strlen(this->base)], "/index.html");
                    else 
                        strcpy(&path[strlen(this->base)], params);
                    
                    fprintf(index, "<html><head>"
                            "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">"
                            "<link href=\"./pure-release-0.6.0/pure.css\" type=\"text/css\" rel=\"stylesheet\">"
                            "<title>YEL's SERVER</title></head>"
                            "<body><h1>Welcome to Yel's Server</h1><table class=\"pure-table pure-table-horizontal\">");
                    fprintf(index, "<thead><tr>"
                            "<th>Parameter #</th><th>Key</th><th>Value</th>"
                            "</tr></thead><tbody>");
                    char *key, *value, *end_token;
                    while(params = strtok_r(NULL, "?&", &end_params)) {
                        fprintf(index, "<tr>");
                        key = strtok_r(params, "=", &end_token);
                        value = strtok_r(NULL, "", &end_token);
                        fprintf(index, "<td>%d</td><td>%s</td><td>%s</td></tr>", counter, key, value);
                        counter++;
                    }
                    fprintf(index, "</tbody></table>"
                            "<script src=\"test.js\"></script>"
                            "</body></html>");
                    fclose(index);
                } else {
                    if(strncmp(request[1], "/", 2) == 0)
                        strcpy(request[1], "/index.html"); 
                    strcpy(&path[strlen(this->base)], request[1]);
                }
                printf("\x1b[32mFILE\x1b[0m: \x1b[35m%s\n\n\x1b[0m", path);
                if((fd = open(path, O_RDONLY)) != -1) { // File found 
                    send(clients[id], "HTTP/1.1 200 OK\n\n", 17, 0);
                    while((bytes_read = read(fd, data_to_send, BYTES)) > 0)
                        write(clients[id], data_to_send, bytes_read);
                }
                else 
                    write(clients[id], "HTTP/1.1 404 Not Found\n", 23); //FILE NOT FOUND
            }
        }
    }
    //Closing connections
    shutdown(clients[id], SHUT_RDWR); // Disable client connection
    close(clients[id]);
    clients[id] = -1;
}
