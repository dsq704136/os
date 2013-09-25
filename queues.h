#ifndef QUEUES_H_INCLUDED
#define QUEUES_H_INCLUDED

#include   "process.h"

typedef struct{
    long                length;
    PCB                 *head;
} PQueue;

PQueue                *timer_queue;
PQueue                *ready_queue;

void InitQueue( PQueue ** );
void AddtoQueue( PQueue *, PCB *, int );
int RemoveFromQueue( PQueue *, PCB * );


#endif // QUEUES_H_INCLUDED
