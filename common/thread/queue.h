/* queue.h
** Copyright 2000 Daniel Robbins, Gentoo Technologies, Inc.
** Author: Daniel Robbins
** Date: 16 Jun 2000
*/
#ifndef QUEUE_H
#define QUEUE_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct node {
  struct node *next;
} node;

typedef struct {
  node *head, *tail; 
  int count;    //count of node
} queue;

void queue_init(queue *myroot);
void queue_put(queue *myroot, node *mynode);
node *queue_get(queue *myroot);

#ifdef __cplusplus
}
#endif

#endif /*QUEUE_H*/
