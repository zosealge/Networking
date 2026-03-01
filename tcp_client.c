#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <sys/poll.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <errno.h>
#include <time.h>

#define SERVER_PORT  9898

#define TRUE             1
#define FALSE            0

long wait_sec(long sec);

int main(void)
{
    char username[8];
    //char buffer[80];
    struct sockaddr_in addr;
    //int close_conn;
    int sock_fd;
    int rc, count=0;
    sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    addr.sin_family=AF_INET;
    addr.sin_port=htons(SERVER_PORT);
    addr.sin_addr.s_addr=inet_addr("127.0.0.1");
    if(sock_fd < 0)
    {
        perror("socket() failed");
        exit(-1);
    }
    printf("Provide username\n");
    fgets(username,8,stdin);
    rc = connect(sock_fd,(struct sockaddr*)&addr,sizeof(addr));
    if(rc < 0)
    {
        perror("connect() failed");
        exit(-1);
    }
    do
    {
        //fgets(buffer,sizeof(buffer),stdin);
        send(sock_fd,username,sizeof(username),0);
        printf("Sent!\n");
        //recv(sock_fd,buffer,sizeof(buffer),0);
        //printf("Received %3d times: %s",count,buffer);
        wait_sec(1);
        count++;
    }
    while(1);
    close(sock_fd);
}

long wait_sec(long sec)
{
    long a;
    time_t start = clock();
    nanosleep((const struct timespec[]){{sec, 0}}, NULL);
    time_t end = clock();
    return a=end-start;
}