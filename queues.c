#include                 <string.h>
#include                 <stdio.h>
#include                 <stdlib.h>
#include                 "global.h"
#include                 "z502.h"
#include                 "syscalls.h"
#include                 "process.h"
#include                 "protos.h"
#include                 "queues.h"



void InitQueue( PQueue **q ){
    PQueue *tmp_q;
    tmp_q = (PQueue *) calloc(1, sizeof(PQueue));
    tmp_q->length = 0;
    tmp_q->head = NULL;
    *q = tmp_q;
}

void AddtoQueue( PQueue *q, PCB *pcb, int orderBy ){

    //TODO Thread Safe
    PCB *p = q->head;
    PCB *pre = NULL;

    if( q->head == NULL ){
        q->head = pcb;
    }
    else{

        // Order by time_of_delay
        if( orderBy == ORDER_TIME_OF_DELAY )
            while( p && p->time_of_delay < pcb->time_of_delay ){
                pre = p;
                p = p->next_pcb;
            }

        else if( orderBy == ORDER_PRIORITY )
            while( p && p->priority < pcb->priority ){
                pre = p;
                p = p->next_pcb;
            }

        else
            while( p ){
                pre = p;
                p = p->next_pcb;
            }

        if( pre == NULL ){
            q->head = pcb;
            pcb->next_pcb = p;
        }
        else{
            pre->next_pcb = pcb;
            pcb->next_pcb = p;
        }
    }
    q->length++;
    printf( "process '%s' already ADD into queue, queue length: %ld\n", pcb->name, q->length );
}

int RemoveFromQueue( PQueue *q, PCB *pcb ){

    PCB *p = q->head;
    PCB *pre = NULL;

    if( p == NULL ){
        printf( "Error: Empty Queue." );
        return -1;
    }

    while( p != NULL ){

        if( p == pcb ){
            if( q->head == p ){
                q->head = p->next_pcb;
                p->next_pcb = NULL;
            }
            else if( p->next_pcb == NULL ){
                pre->next_pcb = NULL;
            }
            else{
                pre->next_pcb = p->next_pcb;
                p->next_pcb = NULL;
            }
            q->length--;
            printf( "process '%s' already REMOVE from queue, queue length: %ld\n", pcb->name, q->length );
            return 1;
        }
        pre = p;
        p = p->next_pcb;
    }
    return 0;
}
