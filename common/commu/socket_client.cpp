#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <netinet/in.h>
//#include <arpa/inet.h>
//#include <errno.h>
#include <poll.h>
#include <netdb.h>

#include "socket_client.h"
#include "commu_disptchr.h"l
#include "device/socket_device.h"
#include "timer_cstm.h"

/*!
    Input:  max -- maximum connection be established
*/
SocketClient::SocketClient(int max) : SocketBase(max)
{
    clnt_inf_ = new ClientInfo[max+1];  //多出的1个 为了定位时 与 SocketServer 保持一致
    for (int i=0; i<=max; i++) {
        clnt_inf_[i].sock_sn = 0;
        clnt_inf_[i].recnct_intvl = 0;
        clnt_inf_[i].tmr_rcnct_ = new TimerCstm;
    }
    
    sn_acc_ = 0;
}

SocketClient::~SocketClient()
{
    for (int i=0; i<=max_connect_; i++) {
        delete clnt_inf_[i].tmr_rcnct_;
    }
    delete [] clnt_inf_;
}

/*!
Find the socket it's sn is minimum

    Return: index of socket be found
*/
int SocketClient::FindMinSn()
{
    int min = sn_acc_;
    int n = 1;
    for (int i=1; i<=max_connect_; i++) {
        if (clnt_inf_[i].sock_sn<min) {
            n = i;
            min = clnt_inf_[i].sock_sn;
        }
    }
    return n;
}

/*!
Start socket client

    Input:  name -- port number or filename
            hostname -- hostname or ip. only for internet
            phy_t -- physical layer protocol
            app_t -- application layer protocol
            intvl -- auto reconnect interval time. unit:s, 0 = not auto reconnect
    Return: index of pl_fd_(range:1-max_connect_), or <=0 in case of error
*/
int SocketClient::Start (const char *name, const char *hostname, int phy_t, int app_t, int intvl)
{
    int sock = Connect(name, hostname);
    if (sock > 0) {
        int k = CreateCommuObj(sock, phy_t, app_t);
        if (k<1) {
            int i = FindMinSn();
            clnt_inf_[i].recnct_intvl = 0;
            DeleteCommuObj(i);
            k = CreateCommuObj(sock, phy_t, app_t);
        }
        if (k>0) {
            strncpy(clnt_inf_[k].name, name, sizeof(clnt_inf_[k].name));
            strncpy(clnt_inf_[k].hostname, hostname, sizeof(clnt_inf_[k].hostname));
            clnt_inf_[k].phy_prtcl = phy_t;
            clnt_inf_[k].app_prtcl = app_t;
            clnt_inf_[k].recnct_intvl = intvl;
            clnt_inf_[k].tmr_rcnct_->Start(intvl);
            clnt_inf_[k].sock_sn = ++sn_acc_;
        } else {
            close(sock);
        }
        sock = k;
    }
    return sock;
}

/*!
Resume socket client connection

    Input:  idx -- index of connection of socket client(range:1-max_connect_)
*/
void SocketClient::Resume(int idx)
{
    int sock = Connect(clnt_inf_[idx].name, clnt_inf_[idx].hostname);
    if (sock > 0) {
        int k = CreateCommuObj(sock, clnt_inf_[idx].phy_t, clnt_inf_[idx].app_t);
        if (k<1) {
            close(sock);
        }
    }
    clnt_inf_[idx].tmr_rcnct_->Start(clnt_inf_[idx].recnct_intvl);
}

/*!
End socket client connection

    Input:  idx -- index of connection of socket client(range:1-max_connect_)
*/
void SocketClient::End(int idx)
{
    clnt_inf_[idx].recnct_intvl = 0;
    DeleteCommuObj(idx);
}

/*!
    Input:  timeout -- block time, unit:ms
    Return: 0=success
*/
int SocketClient::Run(int timeout)
{
    int n = poll(pl_fd_, max_connect_+1, timeout);

    for (int i=1; i<=max_connect_; i++) {
        if (n>0 && pl_fd_[i].revents>0) {
            int k = 0;
            if (pl_fd_[i].revents & POLLRDNORM ) {
                k = commu_dispch_[i]->CeiveTrans();
            }
            if (pl_fd_[i].revents&POLLERR) {
                printf("revents is error\n");
                k = 2;
            }
            if (k==2) {
                DeleteCommuObj(i);
            }
            n--;
        }
        if (pl_fd_[i].fd>0 ) {
            commu_dispch_[i]->Transmit();
            commu_dispch_[i]->PostProcess();
            clnt_inf_[i].tmr_rcnct_->Start(clnt_inf_[i].recnct_intvl);
        } else if (clnt_inf_[i].recnct_intvl>0) {
            if (clnt_inf_[i].tmr_rcnct_->TimeOut()) {
                Resume(i);
            }
        }
    }
    return 0;
}

/*!
Connect to socket server

    Input:  name -- port number or filename
            hostname -- hostname or ip. only for internet
    Return: the file descriptor for the listen socket, or <0 in case of error
*/
int SocketClient::Connect (const char *name, const char *hostname)
{
    int sock, sz;
    sockaddr_in addr_in;
    sockaddr_un addr_un;
    sockaddr *paddr;
    if ( IsInt(name) ) {
        sock = InitSocket( &addr_in, hostname, atoi(name) );
        paddr = (sockaddr*)&addr_in;
        sz = sizeof(addr_in);
        sock_type_ = 0;
    } else {
        sock = InitSocket( &addr_un, name );
        paddr = (sockaddr*)&addr_un;
        sz = sizeof(addr_un);
        sock_type_ = 1;
    }
    if (sock<0) return -1;

    if (0 > connect (sock, paddr, sz)) {
        perror ("connect (client)");
        close(sock);
        return -2;
    }
    
    return sock;
}

/*!
Initialize internet socket used by client
    
    Return: the file descriptor for the new socket, or -1 in case of error
*/
int SocketClient::InitSocket (struct sockaddr_in *name, const char *hostname, uint16_t port)
{
    int sock = socket (PF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror ("socket (client)");
        return -1;
    }
    
    name->sin_family = AF_INET;
    name->sin_port = htons (port);
    struct hostent *hostinfo = gethostbyname (hostname);
    if (hostinfo == NULL) {
        fprintf (stderr, "Unknown host %s.\n", hostname);
        return -1;
    }
    name->sin_addr = *(struct in_addr *) hostinfo->h_addr;
    
    return sock;
}

/*!
Initialize local socket used by client
    
    Return: the file descriptor for the new socket, or -1 in case of error
*/
int SocketClient::InitSocket (struct sockaddr_un *name, const char *filename)
{
    int sock = socket (PF_LOCAL, SOCK_STREAM, 0);
    if (sock < 0) {
        perror ("socket (client)");
        return -1;
    }
    
    name->sun_family = AF_LOCAL;
    strcpy (name->sun_path, filename);
    
    return sock;
}
