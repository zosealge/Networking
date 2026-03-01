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
#include <unistd.h>
#include <time.h>

//          additional hardcoded settings
#define BUFFER_SIZE 128
#define BUFFER_SIZE_SMALL 64
#define HOST_INFO_HOSTNAME_MAX 36
#define MAX_CONNECTIONS 5 // for start
#define FRAME_DURATION 16000000

//          structs
struct host_info
{
    uint16_t tcp_port;
    uint16_t udp_port;
    char hostname[HOST_INFO_HOSTNAME_MAX];
};

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
    time_t fps;
    struct sockaddr_in  addr;
    struct host_info   *host;
    char buffer[BUFFER_SIZE];
    char proc_buffer[BUFFER_SIZE];
    //char send_buffer[BUFFER_SIZE];
    ssize_t recsize;
    socklen_t fromlen = sizeof(addr);

    // configuration

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

    listen_sd = socket(PF_INET,SOCK_DGRAM,IPPROTO_UDP);
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
    do
    {
        memset(buffer,0,sizeof(buffer));
        memset(proc_buffer,0,sizeof(proc_buffer));
        //memset(send_buffer,0,sizeof(send_buffer));
        recsize = recvfrom(listen_sd,buffer,sizeof(buffer),0,(struct sockaddr*)&addr, &fromlen);
        if(recsize < 0)
        {
            printf("recv() function failed... exiting");
            close_conn=true;
        }
        strncpy(proc_buffer,buffer,BUFFER_SIZE);
        //strncpy(send_buffer,"Hello ",6);
        //strncat(send_buffer,proc_buffer,BUFFER_SIZE_SMALL);

        // Buffer input processing ahead
        printf("> %s using %ld bytes\n",buffer, recsize);
        fps=wait_nano(FRAME_DURATION);
        printf("time passed: %jd\n",fps);

    }
    while (close_conn==false);
    close(listen_sd);
    free(host);
    return 0;
}