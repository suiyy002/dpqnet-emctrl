#include <stdlib.h>
#include <string.h>
#include <stdio.h>
//#include <zlib.h>

#include "messageq_guis.h"

MessageQGuiS & messageq_guis()
{
    static MessageQGuiS msgq;
    return msgq;
}


MessageQGuiS::~MessageQGuiS()
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
void MessageQGuiS::InitQGui(int max)
{
    InitQueue(max);
}

/*!
push command into all queues

    Input:  type -- gui communication command
            size -- size of data in bytes
            data -- data be send to
            idx -- ld index or channel index
    Return: 0=success, -1=failure
*/
int MessageQGuiS::Push(int type, uint32_t size, void *data, uint8_t *idx)
{
    int sz = size + 4;
    if (idx) sz += 1;
    char buf[sz];
    memcpy(buf, &sz, 4);
    if (idx) buf[4] = *idx;
    memcpy(&buf[5], data, size);
    int retv;
    for (int i=0; i<queue_max_; i++) {
        retv = PushQueue(i, kGUICommuCmd, type, buf);
        if (retv<0) break;
    }
    return retv;
}
