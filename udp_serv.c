/*
    2026 Metro UDP server
    TCP IPv4
    Testing multiple connection from multiple clients at the same time
    Aiming for UDP packets control

    host need to operate in around 60FPS
    0.016 seconds = 1 frame in framebuffer
    0.016 = 16000000 nano seconds
*/

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <time.h>

//          additional hardcoded settings
#define BUFFER_SIZE 128
#define BUFFER_SIZE_SMALL 64
#define HOST_INFO_HOSTNAME_MAX 36
#define MAX_CONNECTIONS 5 // for start
#define FRAME_DURATION 16000000
#define USERNAME_MAX 8
#define MAX_RCV_DGRAM 16
#define MAGIC_NUM 32

//          structs and globals
struct host_info
{
    uint16_t tcp_port;
    uint16_t udp_port;
    char hostname[HOST_INFO_HOSTNAME_MAX];
};

struct dgram_read
{
    uint8_t magic_number;
    char username[8];
    uint16_t y;
    uint16_t x;
    uint16_t dir;
};

time_t fps;
const char *MAGIC="#";

//          additional header files
#include "src/conf.h"
#include "src/loop.h"

//          prototypes
time_t wait_nano(long nano);

int main(void)
{
    const char *conf_file="config/hostconf.ini";
    int rc;
    int listen_sd;
    bool close_conn=true;
    struct sockaddr_in  addr;
    struct sockaddr_in  peer_addr;
    struct host_info   *host;
    struct dgram_read *mem_buffer;
    struct dgram_read peer_buffer;
    //char buffer[BUFFER_SIZE];
    //char proc_buffer[BUFFER_SIZE];
    //char send_buffer[BUFFER_SIZE];
    ssize_t recsize, max_recsize;
    socklen_t fromlen = sizeof(peer_addr);
    max_recsize=MAX_RCV_DGRAM;
    // configuration
    mem_buffer=malloc(sizeof(struct dgram_read));
    if(mem_buffer == NULL)
    {
        perror("memory allocation failed");
        exit(-1);
    }
    host=malloc(sizeof(struct host_info));
    if(host == NULL)
    {
        perror("memory allocation failed");
        exit(-1);
    }

    rc = config_read_host(host,conf_file);
    if(rc < 0)
    {
        perror("hostconf.ini not found");
        exit(-1);
    }

    // setting socket and bind
    memset(&addr,0,sizeof(addr));
    addr.sin_family         = AF_INET;
    addr.sin_addr.s_addr    = htonl(INADDR_ANY);
    addr.sin_port           = htons(host->udp_port);

    listen_sd = socket(AF_INET,SOCK_DGRAM,IPPROTO_UDP);
    if(listen_sd < 0)
    {
        perror("socket function failed");
        exit(-1);
    }

    rc = bind(listen_sd,(struct sockaddr*)&addr,sizeof(addr));
    if(rc < 0)
    {
        perror("bind function failed");
        close(listen_sd);
        exit(-1);
    }
    // welcome Screen
    printf("\nMetro server initiation complete\n");
    printf("Host now accepting connections...\n");
    printf("UDP Port: %hd\n\n",host->udp_port);
    close_conn=false;

            no_connect: fps=wait_nano(FRAME_DURATION);
            //useless but for adoiving compiler error
    //get connected peer info
    recsize = recvfrom(listen_sd,&peer_buffer,sizeof(peer_buffer),0,(struct sockaddr*)&peer_addr, &fromlen);
        if(recsize < 0)
        {
            printf("recvfrom() function failed... exiting");
            close_conn=true;
        }
        printf("%s : %d connected!\n",inet_ntoa(peer_addr.sin_addr),ntohs(peer_addr.sin_port));
        if(recsize==max_recsize)
        {
            printf("packet size is correct, continue to receive data\n");
        }
        else
        {
            printf("packet size not correct\n");
            printf("host expected %zu\n",max_recsize);
            goto no_connect;
        }
    do
    {
        memset(&peer_buffer,0,sizeof(struct dgram_read));
        recsize = recvfrom(listen_sd,&peer_buffer,sizeof(peer_buffer),0,(struct sockaddr*)&peer_addr, &fromlen);
        if(recsize < 0)
        {
            printf("recvfrom() function failed... exiting");
            close_conn=true;
        }
        if(peer_buffer.magic_number==MAGIC_NUM)
        {
        //strncpy(proc_buffer,buffer,BUFFER_SIZE);
        //strncpy(sepeer_buffer,"Hello ",6);
        //strncat(sepeer_buffer,proc_buffer,BUFFER_SIZE_SMALL);

        // Buffer input processing ahead
        //printf("> %s using %ld bytes",buffer, recsize);
        //set the buffor in the memory for packet loss control
        mem_buffer->y=peer_buffer.y;
        mem_buffer->x=peer_buffer.x;
        mem_buffer->dir=peer_buffer.dir;
        strncpy(mem_buffer->username,peer_buffer.username,USERNAME_MAX);
        //printf("received:\ny: %d\nx: %d\ndir: %d ",mem_buffer->y,mem_buffer->x,mem_buffer->dir);
        printf("received: %zu bytes\ny: %d\nx: %d\ndir: %d ",recsize,peer_buffer.y,peer_buffer.x,peer_buffer.dir);
        printf(" from %s : %d\n",inet_ntoa(peer_addr.sin_addr),ntohs(peer_addr.sin_port));
        printf("sender username: %s\n",peer_buffer.username);
        //fps=wait_nano(FRAME_DURATION);
        //printf("time passed: %jd\n",fps);
        }
        else
        {
            printf("connection datagram not compatible\n");
            printf("received %zu bytes\n",recsize);
        }
    }
    while (close_conn==false);
    close(listen_sd);
    free(host);
    free(mem_buffer);
    return 0;
}