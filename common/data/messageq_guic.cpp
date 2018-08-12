#include <stdlib.h>
#include <string.h>
#include <stdio.h>
//#include <zlib.h>

#include "messageq_guic.h"

MessageQGuiC & messageq_guic()
{
    static MessageQGuiC msgq;
    return msgq;
}


MessageQGuiC::~MessageQGuiC()
{
    int max = queue_max_;
    for (int i=0; i<max; i++) {
        delete [] wait_cnt_[i];
    }
    delete [] wait_cnt_;
}

/*!
Initialize queue

    Input:  max -- maximum number of queue
*/
void MessageQGuiC::InitQGui(int max)
{
    InitQueue(max);
    wait_cnt_ = new uint8_t*[max]
    for (int i=0; i<max; i++) {
        wait_cnt_[i] = new uint8_t[kCmdBufNum];
    }
}

/*!
push command into all queues

    Input:  qidx -- index of queue
            type -- gui communication command
            size -- size of data in bytes
            data -- data be send to
            idx -- ld index or channel index
    Return: 0=success, -1=failure
*/
int MessageQGuiC::Push(uint8_t qidx, int type, uint32_t size, void *data, uint8_t *idx)
{
    int sz = size + 4;
    if (idx) sz += 1;
    char buf[sz];
    memcpy(buf, &sz, 4);
    if (idx) buf[4] = *idx;
    memcpy(&buf[5], data, size);
    return PushQueue(qidx, kGUICommuCmd, type, buf);
}

/*!
Clear the value of waiting counter.
    Input:  idx -- queues index
            sn -- waiting counter serial number in one queue
*/
void MessageQGuiC::ClearWaitCnt(int idx, int sn)
{
    if (idx>=queue_max_||idx<0) return;
    pthread_mutex_lock(&queues_[idx].control.mutex);
    wait_cnt_[idx][sn] = 0;
    pthread_mutex_unlock(&queues_[idx].control.mutex);
}

/*!
Increase waiting counter
*/
void MessageQGuiC::IncreaseCount()
{
    for (int i=0; i<queue_max_; i++) {
        if (queue_state_[i]) {
            pthread_mutex_lock(&queues_[i].control.mutex);
            for (int j=0; j<kCmdBufNum; i++) {
                wait_cnt_[i][j]++;
            }
            pthread_mutex_unlock(&queues_[i].control.mutex);
        }
    }
}

/*!
Fetch all parameter from GUI server
*/
void MessageQGuiC::FetchParam()
{
    uint8_t buf;

    for (int i=0; i<queue_max_; i++) {
        if (queue_state_[i]) continue;
        buf = 0;
        Push(i, kCmdPrmPhd, 1, &buf);
        for (uint8_t j=0; j<kChannelTol; j++) {
            Push(i, kCmdPrmLd, 1, &buf, &j);
            Push(i, kCmdPrmChnl, 1, &buf, &j);
        }
    }
}

/*!
Push control signal command

    Input:  qidx -- index of queue
            type -- 0=Manual trigger record wave
                    1=10cycle data save
                    2= setting gps pulse parameter
                    3=calibrate precision
                    4=get dc component
                    5=reset passwd
                    6=reset transient record
                    7=set time
                    8=reset system
            val1 -- 0: 1=start, 0=stop
                    1: 1=start, 0=stop
                    2: bit0-1 is type, 0=分脉冲,1=秒脉冲,2=使能; bit2-15 is 对时间隔或开关(0=关, 1=开)
                    3: 0=stop, 1=start, 2=get state
                    4: 0=stop, 1=start, 2=get state
                    7: high 2 bytes of time in time_t
                    8: 0=restore to default parameter, 1=initailize system
            val2 -- 7: low 2byte of time in time_t
*/
void MessageQGuiC::PushCtrlSig(uint8_t qidx, uint16_t type, uint16_t val1, uint16_t val2)
{ 
    uint16_t *buf = new uint16_t[3];
    buf[0] = type;
    buf[1] = val1;
    buf[2] = val2;
    
    Push(qidx, kCmdSendCtrlSig, 3, buf);
}
