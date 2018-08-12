/* control.c
** Copyright 2000 Daniel Robbins, Gentoo Technologies, Inc.
** Author: Daniel Robbins
** Date: 16 Jun 2000
**
**
** data_control structs 包含一个int变量"active".  这个变量被用于特殊
** 的多线程设计,此处，每个线程在锁住互斥锁mutex时，检查"active"的状态。
** 如果active=QUITCMD, 线程结束，如果active=1, 线程继续执行任务。这样，通过
** 把active设为QUITCMD, 一个控制线程可以方便地通知另外一个线程终止任务并退出。
*/

#include <pthread.h>
#include "control.h"
#include "pthread_mng.h"

//初始化 data_control结构变量 mycontrol
int control_init(data_control *mycontrol)
{
    if (pthread_mutex_init(&(mycontrol->mutex), NULL)) //初始化互斥锁mutex
        return 1;
    if (pthread_cond_init(&(mycontrol->cond), NULL)) //初始化条件变量cond
        return 1;
    mycontrol->active = 0;
    return 0;
}

//清除 data_control结构变量 mycontrol
int control_destroy(data_control *mycontrol)
{
    if (pthread_cond_destroy(&(mycontrol->cond))) //清除条件变量cond
        return 1;
    if (pthread_mutex_destroy(&(mycontrol->mutex))) //清除互斥锁mutex
        return 1;
    mycontrol->active = 0;
    return 0;
}

//唤醒所有阻塞于条件变量cond的线程
int control_activate(data_control *mycontrol)
{
    if (pthread_mutex_lock(&(mycontrol->mutex)))
        return 0;
    mycontrol->active = 1;
    pthread_mutex_unlock(&(mycontrol->mutex));
    pthread_cond_broadcast(&(mycontrol->cond));
    return 1;
}

//唤醒所有阻塞于条件变量cond的线程，并结束
int control_deactivate(data_control *mycontrol)
{
    if (pthread_mutex_lock(&(mycontrol->mutex)))
        return 0;
    mycontrol->active = QUITCMD;
    pthread_mutex_unlock(&(mycontrol->mutex));
    pthread_cond_broadcast(&(mycontrol->cond));
    return 1;
}

/*!
Description:向线程发送消息

    Input:  type -- 线程类型
            major -- 消息主类
            minor -- 消息次类
    Return: 0=success, -1=failure
*/
int notice_pthread(ThreadType type, int major, int minor, void *point)
{
    WorkNode *to_node = (WorkNode*)malloc(sizeof(WorkNode));
    if (!to_node) {
        printf("ouch! can't malloc!\n");
        return -1;
    }
    to_node->major_type = major;
    to_node->minor_type = minor;
    to_node->point = point;
    switch (type) {
        case kTTDisplay:
            pthread_mutex_lock(&disq.control.mutex);
            queue_put(&disq.task, (node *)to_node);
            pthread_cond_signal(&disq.control.cond);
            pthread_mutex_unlock(&disq.control.mutex);
            break;
        case kTTSave:
            pthread_mutex_lock(&saveq.control.mutex);
            queue_put(&saveq.task, (node *)to_node);
            pthread_cond_signal(&saveq.control.cond);
            pthread_mutex_unlock(&saveq.control.mutex);
            break;
        case kTTMain:
            pthread_mutex_lock(&cwq.control.mutex);
            queue_put(&cwq.task, (node *)to_node);
            pthread_cond_signal(&cwq.control.cond);
            pthread_mutex_unlock(&cwq.control.mutex);
            break;
        default:
            break;
    }
    return 0;
}

/*!
Description:send signal to cleanup queue

    Input:  pthnode -- pthread node will be cleanup
*/
void notice_clrq(void * pthnode)
{
    pthread_mutex_lock(&clrq.control.mutex);
    queue_put(&clrq.task, (node *) pthnode);
    pthread_mutex_unlock(&clrq.control.mutex);
    pthread_cond_signal(&clrq.control.cond);
    printf("thread %d--shutting down...\n", ((CleanupNode*)pthnode)->threadnum);
}

