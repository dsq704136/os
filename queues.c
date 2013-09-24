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

void AddtoQueue( PQueue *q, PCB *pcb ){

    //TODO Thread Safe
    PCB *head = q->head;
    PCB *p_next;

    if( head == NULL ){
        head = pcb;
    }
    else{
        p_next = head->next_pcb;
        // Order by time_of_delay
        while( p_next != NULL && p_next->time_of_delay > pcb->time_of_delay )
            p_next = p_next->next_pcb;

        pcb->next_pcb = p_next;
        p_next = pcb;
    }

    q->length++;
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
            return 1;
        }
        pre = p;
        p = p->next_pcb;
    }
    return 0;
}
