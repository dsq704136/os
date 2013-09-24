#ifndef QUEUES_H_INCLUDED
#define QUEUES_H_INCLUDED

#include   "process.h"

typedef struct{
    long                length;
    PCB                 *head;
} PQueue;

void InitQueue( PQueue ** );
void AddtoQueue( PQueue *, PCB * );
int RemoveFromQueue( PQueue *, PCB * );


#endif // QUEUES_H_INCLUDED
