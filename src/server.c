#define _POSIX_C_SOURCE 200112L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/socket.h>

static void die(const char* msg){ perror(msg); exit(EXIT_FAILURE); }

static int read_line(int fd, char *buf, size_t cap){
    size_t n = 0;
    while(n + 1 < cap){
        char c; ssize_t r = recv(fd, &c, 1, 0);
        if(r == 0) return 0;         // peer closed
        if(r < 0){ if(errno==EINTR) continue; return -1; }
        if(c == '\n'){ buf[n] = 0; return (int)n; }
        buf[n++] = c;
    }
    buf[n] = 0; return (int)n;
}

int main(int argc, char **argv){
    int opt; const char *port = NULL;
    while((opt = getopt(argc, argv, "p:h")) != -1){
        if(opt=='p') port = optarg;
        else { fprintf(stderr,"Usage: %s -p PORT\n", argv[0]); return 2; }
    }
    if(!port){ fprintf(stderr,"Usage: %s -p PORT\n", argv[0]); return 2; }

    struct addrinfo hints = {0}, *res = NULL;
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    if(getaddrinfo(NULL, port, &hints, &res)!=0) die("getaddrinfo");

    int srv = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if(srv<0) die("socket");

    int yes=1; setsockopt(srv, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));
    if(bind(srv, res->ai_addr, res->ai_addrlen)<0) die("bind");
    if(listen(srv, 16)<0) die("listen");
    freeaddrinfo(res);

    printf("[server] listening on port %s\n", port); fflush(stdout);
    srand((unsigned)time(NULL));

    for(;;){
        struct sockaddr_in cli; socklen_t clilen=sizeof(cli);
        int fd = accept(srv, (struct sockaddr*)&cli, &clilen);
        if(fd<0){ if(errno==EINTR) continue; die("accept"); }

        char ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &cli.sin_addr, ip, sizeof(ip));
        unsigned short cport = ntohs(cli.sin_port);

        int target = 1 + rand()%100;
        int attempts = 0;

        printf("%s:%u: CONNECT\n", ip, cport); fflush(stdout);
        const char *hello = "GUESS 1..100\n";
        send(fd, hello, strlen(hello), 0);

        char line[128];
        while(1){
            int r = read_line(fd, line, sizeof(line));
            if(r == 0){ printf("%s:%u: DISCONNECT\n", ip, cport); break; }
            if(r < 0){ perror("recv"); break; }

            attempts++;
            printf("%s:%u:%s\n", ip, cport, line); fflush(stdout);

            char *end=NULL; long guess = strtol(line, &end, 10);
            if(end==line){ const char *err="ERR not a number\n"; send(fd, err, strlen(err), 0); continue; }

            if(guess < target){ const char *ans="LESS\n";    send(fd, ans, strlen(ans), 0); }
            else if(guess > target){ const char *ans="GREATER\n"; send(fd, ans, strlen(ans), 0); }
            else {
                char ok[64]; snprintf(ok, sizeof(ok), "EQUAL %d\n", attempts);
                send(fd, ok, strlen(ok), 0);
                target = 1 + rand()%100; attempts = 0;
                const char *again="NEW 1..100\n";
                send(fd, again, strlen(again), 0);
            }
        }
        close(fd);
    }
}
