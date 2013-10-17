/*********************************************************************

    queues.h

    Author : Shuoqing Ding
    Date   : 2013/10/14

    Define PQueue structure and some related global variables.
    PQueue structure is only used for PCB, all of Queues( timerQ,
    readyQ, suspendQ ) use the same structure.

*********************************************************************/


#ifndef QUEUES_H_INCLUDED
#define QUEUES_H_INCLUDED

#include   "process.h"

typedef struct{
    long                length;
    char                name[32];
    PCB                 *head;         //Point to the first PCB
} PQueue;

PQueue                *timer_queue;
PQueue                *ready_queue;
PQueue                *suspend_queue;

void InitQueue( PQueue **, char * );
void AddtoQueue( PQueue *, PCB *, int );
BOOL IsEmptyQueue( PQueue * );
BOOL HasPCB( PQueue*, PCB* );
void GetFirstPCB( PCB **, PQueue * );
void PrintQueue( PQueue * );
int  RemoveFromQueue( PQueue *, PCB * );


#endif // QUEUES_H_INCLUDED
