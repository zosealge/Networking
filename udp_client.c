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
#define USERNAME_MAX 8
#define MAGIC_NUM 32

//          structs and globals
struct host_info
{
    uint16_t udp_port;
    char hostip[HOST_INFO_HOSTNAME_MAX];
};

struct dgram_send
{
    uint8_t magic_number;
    char username[8];
    uint16_t y;
    uint16_t x;
    uint16_t dir; 
};

time_t sendtime;
ssize_t dgram_p;

//          additional header files
#include "src/client_conf_udp.h"
#include "src/loop.h"

//          prototypes
time_t wait_sec(long sec);

int main(void)
{
    const char *conf_file="config/connudp.ini";
    char *debug_name="debug";
    int rc, udp_sock;
    int counter=0;
    char *debug_buf="abcd";
    bool close_conn=true;
    bool debug=false;
    char buf_username[8];
    //char buffer[256];
    struct host_info    *host;
    struct sockaddr_in   addr;
    struct dgram_send   *send;
    //ssize_t recsize;
    send=malloc(sizeof(struct dgram_send));
    if(send == NULL)
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

    udp_sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP); //UDP
    if (udp_sock < 0)
    {
        perror("socket function failed");
        exit(-1);
    }
    memset(send,0,sizeof(struct dgram_send));
    printf("Welcome to the udp_client test\n");
    printf("Provide username... \n >");
    fgets(buf_username,USERNAME_MAX,stdin);
    printf("Go: %s\n",buf_username);
    rc=strncmp(buf_username,debug_name,(USERNAME_MAX-3));
    if(rc==0)
    {
        printf("Debug mode on\n");
        debug=true;
    }
    strncpy(send->username,buf_username,USERNAME_MAX);
    close_conn=false;
    do
    {
        if(debug)
        {
            counter++;
            dgram_p = sendto(udp_sock, debug_buf, sizeof(char), 0, (struct sockaddr*)&addr, sizeof(addr));
            if (dgram_p < 0)
            {
                printf("failed to send data... closing udp_sock\n");
                close_conn=true;
            }
            printf("[Debug]Sent %zu bytes / ASCII: %s\n",dgram_p,debug_buf);
            sendtime=wait_sec(3);
        }
        else
        {
            counter++;
            send->magic_number  =MAGIC_NUM;
            send->y             =counter;
            send->x             =counter*2;
            send->dir           =counter*4;
            dgram_p = sendto(udp_sock, send, sizeof(*send), 0, (struct sockaddr*)&addr, sizeof(addr));
            if (dgram_p < 0)
            {
                printf("failed to send data... closing udp_sock\n");
                close_conn=true;
            }
            else
            {
                printf("Sent %zu bytes!\n",dgram_p);
                printf("%d / %d / % d\n",send->y,send->x,send->dir);
                memset(send,0,sizeof(struct dgram_send));
                strncpy(send->username,buf_username,USERNAME_MAX);
                sendtime=wait_sec(2);
            }
        }
    }
    while(close_conn==false); 
    close(udp_sock); // close the socket
    free(host);
    free(send);
    return 0;
}