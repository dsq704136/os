#include                 <string.h>
#include                 <stdio.h>
#include                 <stdlib.h>
#include                 "global.h"
#include                 "z502.h"
#include                 "syscalls.h"
#include                 "process.h"
#include                 "protos.h"
#include                 "queues.h"



void InitQueue( PQueue **q, char *name ){

    PQueue *tmp_q;
    tmp_q = (PQueue *) calloc(1, sizeof(PQueue));

    memset( tmp_q->name, '\0', 32 );
    strcpy( tmp_q->name, name );

    tmp_q->length = 0;
    tmp_q->head = NULL;
    *q = tmp_q;
}


BOOL HasPCB( PQueue *q, PCB *pcb ){

    PCB *p = q->head;

    while( p != NULL ){
        if( p->pid == pcb->pid )
            return TRUE;
        p = p->next_pcb;
    }
    return FALSE;
}

BOOL IsEmptyQueue( PQueue *q ){

    if( q->head == NULL )
        return TRUE;

    return FALSE;
}


/************************************************************************
    GetFirstPCB
        Get the first PCB in given queue. First PCB may means the least
        time_of_delay or the smallest priority.
************************************************************************/

void GetFirstPCB( PCB **pcb, PQueue *q ){
    *pcb = q->head;
}


/************************************************************************
    AddtoQueue
        Add PCB into the certain queue by given order and change
        the state of process. If queue is timer queue then order
        by delay_of_time. If queue is ready ready queue then order
        by priority( smaller is greater ).
************************************************************************/

void AddtoQueue( PQueue *q, PCB *pcb, int orderBy ){

    if( HasPCB( q, pcb ) == 1 )
        return;

    PCB *p = q->head;
    PCB *pre = NULL;

    if( q->head == NULL )
        q->head = pcb;

    else{
        // Order by time_of_delay
        if( orderBy == ORDER_TIME_OF_DELAY )
            while( p && p->time_of_delay <= pcb->time_of_delay ){
                pre = p;
                p = p->next_pcb;
            }

        else if( orderBy == ORDER_PRIORITY )
            while( p && p->priority <= pcb->priority ){
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

    if( strcmp( q->name, "Ready" ) == 0 )
        pcb->state = READY;
    else if( strcmp( q->name, "Timer" ) == 0 )
        pcb->state = WAITING;
    else if( strcmp( q->name, "Suspend" ) == 0 )
        pcb->state = SUSPEND;

    q->length++;
    //printf( "ADD '%s' into %s queue(Length: %ld)\n", pcb->name, q->name, q->length );
}

void PrintQueue( PQueue *q ){

    PCB *p = q->head;

    while( p != NULL ){
        printf( "%s(%d), ",p->name, p->pid );
        p = p->next_pcb;
    }
    printf( "\n" );

}

int RemoveFromQueue( PQueue *q, PCB *pcb ){

    PCB *p = q->head;
    PCB *pre = NULL;

    if( IsEmptyQueue( q ) ){
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
            //printf( "REMOVE '%s' from %s queue(Length: %ld)\n", pcb->name, q->name, q->length );
            return 1;
        }
        pre = p;
        p = p->next_pcb;
    }
    return 0;
}
