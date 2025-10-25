#define _POSIX_C_SOURCE 200112L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/socket.h>

static void die(const char* msg){ perror(msg); exit(EXIT_FAILURE); }

static int read_line_sock(int fd, char *buf, size_t cap){
    size_t n = 0;
    while(n + 1 < cap){
        char c; ssize_t r = recv(fd, &c, 1, 0);
        if(r == 0) return 0; if(r < 0){ if(errno==EINTR) continue; return -1; }
        if(c == '\n'){ buf[n]=0; return (int)n; }
        buf[n++]=c;
    }
    buf[n]=0; return (int)n;
}

int main(int argc, char **argv){
    int opt; const char *addr=NULL, *port=NULL;
    while((opt=getopt(argc,argv,"a:p:h"))!=-1){
        if(opt=='a') addr=optarg;
        else if(opt=='p') port=optarg;
        else { fprintf(stderr,"Usage: %s -a ADDR -p PORT\n", argv[0]); return 2; }
    }
    if(!addr || !port){ fprintf(stderr,"Usage: %s -a ADDR -p PORT\n", argv[0]); return 2; }

    struct addrinfo hints={0}, *res=NULL;
    hints.ai_family=AF_INET; hints.ai_socktype=SOCK_STREAM;
    if(getaddrinfo(addr, port, &hints, &res)!=0) die("getaddrinfo");

    int fd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if(fd<0) die("socket");
    if(connect(fd, res->ai_addr, res->ai_addrlen)<0) die("connect");
    freeaddrinfo(res);

    char buf[128];
    int r = read_line_sock(fd, buf, sizeof(buf));
    if(r>0) printf("%s\n", buf);

    while(1){
        printf("your guess> "); fflush(stdout);
        if(!fgets(buf, sizeof(buf), stdin)) break;
        size_t len=strlen(buf); if(len && buf[len-1]=='\n') buf[len-1]=0;
        if(strcmp(buf,"quit")==0 || strcmp(buf,"exit")==0) break;

        char out[128]; snprintf(out, sizeof(out), "%s\n", buf);
        if(send(fd, out, strlen(out), 0) < 0){ perror("send"); break; }

        int rr = read_line_sock(fd, buf, sizeof(buf));
        if(rr<=0){ printf("disconnected\n"); break; }
        printf("server: %s\n", buf);

        if(strncmp(buf,"EQUAL",5)==0){
            int rr2 = read_line_sock(fd, buf, sizeof(buf));
            if(rr2>0) printf("server: %s\n", buf);
        }
    }
    close(fd);
    return 0;
}
