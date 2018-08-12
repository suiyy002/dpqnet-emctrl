#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <poll.h>

#include "socket_server.h"
#include "commu_disptchr.h"
#include "device/socket_device.h"
#include "timer_cstm.h"
#include "time_cst.h"

const int SocketServer::kIdleWTime = 60;

/*!
    Input:  max -- maximum connection be established
            name -- portnumber or filename
*/
SocketServer::SocketServer(int max, const char *name) : SocketBase(max)
{
    strncpy(addr_name_, name, sizeof(addr_name_));

    max++;  //以下[0]不用，为了在定位时，与pollfd保持一致
    timer_idle_ = new TimerCstm[max];
}

SocketServer::~SocketServer()
{
    delete [] timer_idle_;
}

/*!
    Input:  timeout -- block time, unit:ms
    Return: -1=start server failure
*/
int SocketServer::Run(int timeout)
{
    if (pl_fd_[0].fd<0) {
        pl_fd_[0].fd = Start(addr_name_);
        if (pl_fd_[0].fd<0) return -1;
    }

    int n = poll(pl_fd_, max_connect_+1, timeout);
    if (pl_fd_[0].revents & POLLRDNORM ) {  //received events=POLLRDNORM
        int sock = Accept(pl_fd_[0].fd, sock_type_);
        int k = CreateCommuObj(sock, phy_prtcl_, app_prtcl_);
        if (k>0) timer_idle_[k].Start(kIdleWTime);
        n--;
    }

    for (int i=1; i<=max_connect_; i++) {
        if (n>0 && pl_fd_[i].revents>0) {
            int k = 0;
            if (pl_fd_[i].revents & POLLRDNORM ) {
                k = commu_dispch_[i]->CeiveTrans();
                timer_idle_[i].Start(kIdleWTime);
            }
            if (pl_fd_[i].revents&POLLERR) {
                printf("revents is error\n");
                k = 2;
            }
            if (k==2) DeleteCommuObj(i);
            n--;
        }
        if (pl_fd_[i].fd>0 ) {
            commu_dispch_[i]->Transmit();
            commu_dispch_[i]->PostProcess();
            if (timer_idle_[i].TimeOut()) { //idle time out
                printf("socket time out!\n");
                DeleteCommuObj(i);
            }
        }
    }
    return 0;
}


/*!
Make internet socket used by server
    
    Return: the file descriptor for the new socket, or -1 in case of error
*/
int SocketServer::MakeSocket (uint16_t port)
{
    int sock = socket (PF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror ("socket");
        return -1;
    }
    
    int opt = 1;    //Allows the socket to be bound to an port that is already in use
    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    
    struct sockaddr_in name;
    name.sin_family = AF_INET;
    name.sin_port = htons (port);
    name.sin_addr.s_addr = htonl (INADDR_ANY);
    if (bind (sock, (struct sockaddr *) &name, sizeof (name)) < 0) {
        perror ("bind");
        close(sock);
        return -1;
    }
    return sock;
}


/*!
Make local socket by server

    Return: the file descriptor for the new socket, or -1 in case of error
*/
int SocketServer::MakeSocket (const char *filename)
{
    // Create the socket.
    int sock = socket (PF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror ("socket");
        return -1;
    }

    struct sockaddr_un name;
    name.sun_family = AF_LOCAL;
    strncpy (name.sun_path, filename, sizeof(name.sun_path));
    name.sun_path[sizeof(name.sun_path) - 1] = 0;

    size_t size = SUN_LEN (&name);  // offsetof(sockaddr_un, sun_path) + strlen(name.sun_path);
    if (bind (sock, (struct sockaddr *)&name, size) < 0) {
        perror ("bind");
        close(sock);
        return -1;
    }
    return sock;
}

/*!
    Input:  socket -- the file descriptor of listen socket
            type -- socket type. 0=internet, 1=local
    Return:  the descriptor for new connected socket. <0=error
*/
int SocketServer::Accept(int socket, int type)
{
    int sock;
    socklen_t len;
    char stri[64];
    sockaddr_in c_iaddr; //client internet address
    sockaddr_un c_laddr; //client local address
    switch (type) {
        case 0:
            len = sizeof(sockaddr_in);
            sock = accept(socket, (struct sockaddr *)&c_iaddr, &len);
            inet_ntop (AF_INET, &c_iaddr.sin_addr, stri, sizeof(stri));
            printf("connected from %s, port=%d\n", stri, ntohs(c_iaddr.sin_port));
            break;
        case 1:
            len = sizeof(sockaddr_un);
            sock = accept(socket, (struct sockaddr *)&c_laddr, &len);
            printf("connected from %s\n", c_laddr.sun_path);
            break;
        default:
            return -1;
    }
    return sock;
}

#define kReqNum 4  //socket 最多同时处理多少个连接请求
/*!
Start socket server

    Input:  name -- port number or filename
    Return: the file descriptor for the listen socket, or -1 in case of error
*/
int SocketServer::Start (const char *name)
{
    int sock;
    if ( IsInt(name) ) {
        sock = MakeSocket( atoi(name) );
        sock_type_ = 0;
    } else {
        sock = MakeSocket(name);
        sock_type_ = 1;
    }
    if (sock<0) return -1;

    //允许 socket listen fd 接受连接请求，使该socket为Server
    if (listen(sock, kReqNum)) {
        perror("listen");
        close(sock);
        return -1;
    }
    return sock;
}
