/*! \file message_queue.h
    \brief Message queue.
    Copyright (c) 2017  Xi'an Boyuu Electric, Inc.
*/
#ifndef _MESSAGE_QUEUE_H_
#define _MESSAGE_QUEUE_H_

#include <stdint.h>
#include "pthread_mng.h"

class MessageQueue
{
public:
	MessageQueue(int max);
	~MessageQueue();
	
    int BindQueue(int idx);
    void FreeQueue(int idx);
    WorkNode *Pop(int idx);
    
    //Accessors
protected:
    void InitQueue(int max);
    int PushQueue(int idx, int major, int minor, void *point=NULL);

    int queue_max_;   //maximum queue
    PthreadQueue *queues_;
    int *queue_state_;   //0=free, 1=be used
private:
    static const int kMaxNodeNum=32;
};

#endif  //_MESSAGE_QUEUE_H_ 
