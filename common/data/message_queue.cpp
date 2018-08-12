#include <stdlib.h>
#include <string.h>
#include <stdio.h>
//#include <zlib.h>

#include "message_queue.h"

MessageQueue::MessageQueue()
{
    queue_max_ = 0;
}

MessageQueue::~MessageQueue()
{
    delete [] queue_state_;
    int n = queue_max_;
    for (int i=0; i<n; i++) {
        FreeQueue(i);
    }
    delete [] queues_;
}

/*!
Initialize queue

    Input:  max -- maximum number of queue
*/
void MessageQueue::InitQueue(int max)
{
    queue_max_ = max;
    queues_ = new PthreadQueue[max];
    for (int i=0; i<max; i++) {
        if (control_init(&queues_[i].control)) {
            control_destroy(&clrq.control);
            dabort();
        }
        queue_init(&queues_[i].task);
    }
    queue_state_ = new int[max];
    memset(queue_state_, 0, sizeof(int)*max);
}

/*!
bind queue

    Input:  idx -- queue index
    Return: index of mesage queue. -1=failure, queue be using
*/
int MessageQueue::BindQueue(int idx)
{
    if (queue_state_[idx]) return -1;
    queue_state_[idx] = 1;
    return idx;
}

/*!
free queue

    Input:  idx -- queue index
*/
void MessageQueue::FreeQueue(int idx)
{
    if (idx>=queue_max_||idx<0) return;
    pthread_mutex_lock(&queues_[idx].control.mutex);
    WorkNode *nd;
    do {
        nd = (WorkNode *)queue_get(&queues_[idx].task);
        if (nd->point) delete [] nd->point;
        free(nd);
    } while (nd);
    pthread_mutex_unlock(&queues_[idx].control.mutex);
    queue_state_[idx] = 0;
}

/*!
push node into one queue

    Input:  idx -- queue index
            major -- major type of message
            minor -- minor type of message
            point -- data of message. *(uint32_t *)point is size
    Return: 0=success, -1=failure
*/
int MessageQueue::PushQueue(int idx, int major, int minor, void *point)
{
    if (idx>=queue_max_||idx<0) return -1;
    WorkNode *to_node;
    if (queue_state_[idx]) {
        to_node = (WorkNode*)malloc(sizeof(WorkNode));
        if (!to_node) {
            printf("ouch! can't malloc!\n");
            return -1;
        }
        to_node->major_type = major;
        to_node->minor_type = minor;
        to_node->point = point;
        pthread_mutex_lock(&queues_[idx].control.mutex);
        queue_put(&queues_[idx].task, (node *)to_node);
        if (queues_[idx].task.count>kMaxNodeNum) {
            WorkNode *nd = (WorkNode *)queue_get(&queues_[idx].task);
            if (nd->point) delete [] nd->point;
            free(nd);
        }
        pthread_mutex_unlock(&queues_[idx].control.mutex);
    }
    return 0;
}

/*!
pop node from queue

    Input:  idx -- queue index
    Return: node. if no node, return NULL
*/
WorkNode *MessageQueue::Pop(int idx)
{
    if (idx>=queue_max_||idx<0) return NULL;
    pthread_mutex_lock(&queues_[idx].control.mutex);
    return (WorkNode *) queue_get(&queues_[idx].task);
    pthread_mutex_unlock(&queues_[idx].control.mutex);
}



