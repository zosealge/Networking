/*
    2026 Metro server
    TCP IPv4 
    Used IBM i 7.4.0 Socket programming example as a starting point
    https://www.ibm.com/docs/en/i/7.4.0?topic=designs-using-poll-instead-select
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
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

//            additional hardcoded settings
#define HOST_INFO_HOSTNAME_MAX 36
#define MAX_CONNECTIONS 5 // for start

//            structs
struct host_info
{
    uint16_t tcp_port;
    uint16_t udp_port;
    char hostname[HOST_INFO_HOSTNAME_MAX];
};

//            additional header files
#include "src/conf.h"

int main(void)
{
  const char *conf_file="config/hostconf.ini";
  // int   len // for storing lenght of incoming variable
  int    rc, on = 1;
  int    listen_sd = -1, new_sd = -1;
  bool   end_server = false, compress_array = false;
  bool   close_conn;
  char   buffer[80];
  struct sockaddr_in   addr;
  struct host_info    *host;
  int    timeout;
  struct pollfd fds[200];
  int    nfds = 1, current_size = 0, i, j;

// 1. Read configuration
host=malloc(sizeof(struct host_info));
if(host==NULL)
{
    perror("memory allocation failed");
    exit(-1);
}
rc = config_read_host(host,conf_file);
if(rc == -1)
{
    perror("hostconf.ini - not detected");
    exit(-1);
}

listen_sd = socket(AF_INET, SOCK_STREAM, 0);
if(listen_sd < 0)
{
    perror("socket function failed");
    exit(-1);
}

// 2. Set socket options (e.g., SO_REUSEADDR)

rc = setsockopt(listen_sd, SOL_SOCKET, SO_REUSEADDR, (char*)&on, sizeof(on));
if(rc < 0)
{
    perror("socket function (socket option) failed");
    close(listen_sd);
    exit(-1);
}

// 2a. (optional) use ioctl to set socket to be nonblocking.

rc = ioctl(listen_sd, FIONBIO, (char *)&on);
  if (rc < 0)
  {
    perror("ioctl() failed");
    close(listen_sd);
    exit(-1);
  }

// 3. Bind to an address and port
memset(&addr, 0, sizeof(addr));
addr.sin_family         = AF_INET;
addr.sin_addr.s_addr    = htonl(INADDR_ANY);
addr.sin_port           = htons(host->tcp_port);
rc = bind(listen_sd,(struct sockaddr*)&addr,sizeof(addr));
if(rc < 0)
{
    perror("bind function failed");
    close(listen_sd);
    exit(-1);
}

// 4. listen

rc = listen(listen_sd,MAX_CONNECTIONS);
if(rc<0)
{
    perror("listen function failed");
    close(listen_sd);
    exit(-1);
}

// 5. init pollfd structure
memset(fds,0,sizeof(fds));
fds[0].fd = listen_sd;
fds[0].events = POLLIN;
timeout = (3 * 60 * 1000);
// welcome Screen
    printf("\nMetro server initiation complete\n");
    printf("Host now accepting connections...\n");
    printf("TCP Port: %hd\n",host->tcp_port);
do
{
    printf("...waiting...\n");
    rc = poll(fds,nfds,timeout);
    if(rc < 0)
    {
        perror("polling failed");
        break;
    }
    else if(rc == 0)
    {
        printf("host time out... closing\n");
        break;
    }

    current_size=nfds;
    for(i=0;i<current_size;i++)
    {
        if(fds[i].revents == 0)
        continue;
        else if(fds[i].revents != POLLIN)
        {
            printf("revents %d failed\n",fds[i].revents);
            end_server=true;
            break;
        }
        else if(fds[i].fd == listen_sd)
        {
            // listening socket is readable - opening connection ahead
            do
            {
                new_sd=accept(listen_sd,NULL,NULL);
                if(new_sd < 0)
                {
                    if(errno != EWOULDBLOCK)
                    {
                        perror("accept function failed");
                        end_server=true;
                    }
                    break;
                }
                // incoming connection
                fds[nfds].fd = new_sd;
                fds[nfds].events = POLLIN;
                nfds++;
            }
            while(new_sd != -1);
        }
        else
        {
            // descriptor is readable
            close_conn=false;
                // first message contain user name (and probalby more information later)
                // printf connected user name
                rc = recv(fds[i].fd,buffer,sizeof(buffer),0);
                if(rc < 0)
                {
                    if(errno != EWOULDBLOCK)
                    {
                        perror("recv function failed");
                        close_conn=true;
                    }
                    break;
                }
                if(rc == 0)
                {
                    printf("connection closed\n");
                    close_conn=true;
                    break;
                }
                printf("[HOST] %s ...joined chat\n",buffer);
                // user accepted - print name

            do
            {
                rc = recv(fds[i].fd,buffer,sizeof(buffer),0);
                if(rc < 0)
                {
                    if(errno != EWOULDBLOCK)
                    {
                        perror("recv function failed");
                        close_conn=true;
                    }
                    break;
                }
                if(rc == 0)
                {
                    printf("connection closed\n");
                    close_conn=true;
                    break;
                }
                // len = rc; // len stores the byte value received from recv()
                printf(">%s\n",buffer);
            }
            while(true);

            if(close_conn)
            {
                close(fds[i].fd);
                fds[i].fd = -1;
                compress_array=true;
            }
        } // end of existing connection is readable
    }     // end of loop through pollable descriptors

    if (compress_array)
    {
      compress_array=false;
      for (i = 0; i < nfds; i++)
      {
        if (fds[i].fd == -1)
        {
          for(j = i; j < nfds-1; j++)
          {
            fds[j].fd = fds[j+1].fd;
          }
          i--;
          nfds--;
        }
      }
    }
}
while(end_server==false);

for (i = 0; i < nfds; i++)
  {
    if(fds[i].fd >= 0)
      close(fds[i].fd);
  }
free(host);

return 0;
}