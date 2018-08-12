#include <stdio.h>
//#include <stdlib.h>
//#include <string.h>
//#include <sys/socket.h>
//#include <sys/un.h>
//#include <unistd.h>
#include <poll.h>
//#include <errno.h>
#include "commu_disptchr.h"
#include "device/serial_device.h"
#include "timer_cstm.h"
#include "serial_handle.h"

/*!
    Input:  max -- maximum port be handled
*/
SerialHandle::SerialHandle(int max)
{
    max_connect_ = max;
    pl_fd_ = new pollfd[max];
    for (int i=0; i<max; i++) {
        pl_fd_[i].fd = -1;
        pl_fd_[i].events = POLLRDNORM;
    }
    srlpt_num_ = new int[max];
    timer_idle_ = new TimerCstm[max];

    commu_dispch_ = new CommuDisptchr*[max];
    for (int i=0; i<max; i++) {
        commu_dispch_[i] = NULL;
    }
}

SerialHandle::~SerialHandle()
{
    for (int i=0; i<max_connect_; i++) {
        DeleteCommuObj(i);
    }
    delete [] commu_dispch_;
    
    delete [] timer_idle_;
    delete [] srlpt_num_;
    delete [] pl_fd_;
}

/*!
    Input:  timeout -- block time, unit:ms
    Return: 0=success
*/
int SerialHandle::Run(int timeout)
{
    int n = poll(pl_fd_, max_connect_, timeout);
    
    for (int i=0; i<max_connect_; i++) {
        if (n>0 && pl_fd_[i].revents) {
            int k = 0;
            if (pl_fd_[i].revents & POLLRDNORM ) {
                k = commu_dispch_[i]->CeiveTrans();
                timer_idle_[i].Start(300*60);
            }
            if (k==2) {
                printf("serial port read data error!\n");
            }
            n--;
        }
        if (pl_fd_[i].fd>0 ) {
            commu_dispch_[i]->PostProcess();
            if (timer_idle_[i].TimeOut()) { //idle time out
                pl_fd_[i].fd = commu_dispch_[i]->Restart();
            }
        }
    }
    return 0;
}

inline int SerialHandle::FindIdleFD(int sn)
{
    int i;
    for (i=0; i<max_connect_; i++) {
        if (pl_fd_[i].fd<0) continue;
        if (srlpt_num_[i]==sn) break;
    }
    if (i>=max_connect_) {
        for (i=0; i<max_connect_; i++) {
            if (pl_fd_[i].fd<0) break;
        }
    }
    if (i>=max_connect_) return -1;
    return i;
}

/*!
Create communication object for one connection

    Input:  sn -- number of serial port. 0=COM0,1=COM1...
            rate -- baudrate
            phy_t -- physical layer protocol
            app_t -- application layer protocol
*/
void SerialHandle::CreateCommuObj(int sn, int rate, int phy_t, int app_t)
{
    if (sn<0) return;
    int i = FindIdleFD(sn);
    if (i>=0) {
        //printf("Create commu_dispch_[%d] @ %s\n", i, NowTime(0));
        if (commu_dispch_[i]!=NULL) {
            DeleteCommuObj(i);
        }
        commu_dispch_[i] = new CommuDisptchr;
        SerialDevice *dev = new SerialDevice;   //Serial port device object
        pl_fd_[i].fd = dev->Open(sn, rate);
        if (pl_fd_[i].fd<0) {
            printf("Open COM%d @ speed:%d failure!\n", sn, rate);
            delete dev;
            delete commu_dispch_[i];
        } else {
            commu_dispch_[i]->SetAssocObj(dev, phy_t, app_t);
            srlpt_num_[i] = sn;
            timer_idle_[i].Start(300*60);
        }
    }
}

/*!
Delete communication object for one connection

    Input:  idx -- index of communication object. 0=commu_dispch_[0], 0=commu_dispch_[1]
*/
void SerialHandle::DeleteCommuObj(int idx)
{
    if (commu_dispch_[idx]==NULL) return;

    delete commu_dispch_[idx];
    commu_dispch_[idx] = NULL;
    pl_fd_[idx].fd = -1;
}
