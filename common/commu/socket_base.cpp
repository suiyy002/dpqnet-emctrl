#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <errno.h>
#include <poll.h>

#include "socket_base.h"
#include "commu_disptchr.h"
#include "device/socket_device.h"
#include "time_cst.h"

/*!
    Input:  max -- maximum connection be established
*/
SocketBase::SocketBase(int max)
{
    max_connect_ = max;
    pl_fd_ = new pollfd[max+1];  //多出的1个用于server listen, no use for client
    for (int i=0; i<=max; i++) {
        pl_fd_[i].fd = -1;
        pl_fd_[i].events = POLLRDNORM;
    }

    commu_dispch_ = new CommuDisptchr*[max+1];  //多出的1个 为了定位时 与 pl_fd_ 保持一致
    for (int i=0; i<=max; i++) {
        commu_dispch_[i] = NULL;
    }
    sock_type_ = 0;
}

SocketBase::~SocketBase()
{
    for (int i=0; i<=max_connect_; i++) {
        DeleteCommuObj(i);
    }
    delete [] commu_dispch_;

    delete [] pl_fd_;
    if (sock_type_) unlink(addr_name_); // Remove the socket file.
}

bool SocketBase::IsInt(const char *str)
{
    for (int i=0; i<strlen(str); i++) {
        if(!isdigit(str[i])) return false;
    }
    return true;
}

/*!
find idle file description

    Return: index of communication object
*/
inline int SocketBase::FindIdleFD()
{
    int i;
    for (i=1; i<=max_connect_; i++) {
        if (pl_fd_[i].fd<0) break;
    }
    if (i>max_connect_) return -1;
    return i;
}

/*!
Create communication object for one connection

    Input:  sock -- the file descriptor for new connected socket
            phy_t -- physical layer protocol
            app_t -- application layer protocol
    Return: index of pl_fd_(1~max_connect_), or -1 in case of error
*/
int SocketBase::CreateCommuObj(int sock, int phy_t, int app_t)
{
    if (sock<=0) return -1;
    int i = FindIdleFD();
    if (i>0) {
        pl_fd_[i].fd = sock;
        printf("Create commu_dispch_[%d] @ %s\n", i, NowTime(0));
        if (commu_dispch_[i]!=NULL) {
            DeleteCommuObj(i);
        }
        commu_dispch_[i] = new CommuDisptchr;
        
        SocketDevice *dev = new SocketDevice(sock);   //Socket设备对象
        commu_dispch_[i]->SetAssocObj(dev, phy_t, app_t, i-1);
    } else {
        close(sock);
    }
    return i;
}

/*!
Delete communication object for one connection

    Input:  index of communication object. 0=commu_dispch_[0], 0=commu_dispch_[1]
*/
void SocketBase::DeleteCommuObj(int idx)
{
    if (commu_dispch_[idx]==NULL) return;

    delete commu_dispch_[idx];
    commu_dispch_[idx] = NULL;
    pl_fd_[idx].fd = -1;
}


