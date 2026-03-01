#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#include <arpa/inet.h>
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
#define FRAME_DURATION 16000000 // one FPS

//          structs
struct host_info
{
    uint16_t udp_port;
    char hostip[HOST_INFO_HOSTNAME_MAX];
};

//          additional header files
#include "src/client_conf_udp.h"
#include "src/loop.h"

//          prototypes
time_t wait_sec(long sec);

int main(void)
{
    const char *conf_file="config/connudp.ini";
    int rc, udp_sock;
    bool close_conn=true;
    char username[8];
    char buffer[256];
    struct host_info    *host;
    struct sockaddr_in   addr;
    //ssize_t recsize;
    time_t ret_time;

    host=malloc(sizeof(struct host_info));
    if(host == NULL)
    {
        perror("memory allocation failed");
        exit(-1);
    }
    rc = config_read_udp_client(host,conf_file);
    if(rc < 0)
    {
        perror("connudp.ini read error");
        exit(-1);
    }

    // setting socket information and send data
    memset(&addr,0,sizeof(addr));
    addr.sin_family         = AF_INET;
    addr.sin_addr.s_addr    = inet_addr(host->hostip);
    addr.sin_port           = htons(host->udp_port);

    udp_sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP); //UDP
    if (udp_sock < 0)
    {
        perror("socket function failed");
        exit(-1);
    }
    printf("Welcome to the udp_client test\n");
    printf("Provide username... \n >");
    fgets(username,8,stdin);
    printf("Go: %s\n",username);
    strncpy(buffer,username,200);
    close_conn=false;
    do
    {
        rc = sendto(udp_sock, buffer, strlen(buffer), 0, (struct sockaddr*)&addr, sizeof(addr));
        if (rc < 0)
        {
            printf("failed to send data... closing udp_sock\n");
            close_conn=true;
        }
        else
        {
            printf("Sent!\n");
        }
        ret_time=wait_sec(2);
        printf("waited for %ld\n",ret_time);
    }
    while(close_conn==false); 
    close(udp_sock); // close the socket
    return 0;
}