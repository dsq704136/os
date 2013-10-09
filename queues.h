#ifndef QUEUES_H_INCLUDED
#define QUEUES_H_INCLUDED

#include   "process.h"

typedef struct{
    long                length;
    char                name[32];
    PCB                 *head;
} PQueue;

PQueue                *timer_queue;
PQueue                *ready_queue;

void InitQueue( PQueue **, char * );
void AddtoQueue( PQueue *, PCB *, int );
BOOL IsEmptyQueue( PQueue * );
void GetFirstPCB( PCB **, PQueue * );
void PrintQueue( PQueue * );
int RemoveFromQueue( PQueue *, PCB * );


#endif // QUEUES_H_INCLUDED
