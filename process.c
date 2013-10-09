#include                 <string.h>
#include                 <stdio.h>
#include                 <stdlib.h>
#include                 <time.h>
#include                 "global.h"
#include                 "syscalls.h"
#include                 "process.h"
#include                 "queues.h"
#include                 "protos.h"
#include                 "linklist.h"



void OSCreateProcess( PCB **p, void *starting_address, char *name,
                      int priority, BOOL user_or_kernel){

    PCB *new_process = (PCB *) calloc(1, sizeof(PCB));

    //int len_name = strlen( name );
    memset( new_process->name, '\0', 64 );
    strcpy( new_process->name, name );

    new_process->pid = ++global_pid;
    new_process->next_pcb = NULL;
    new_process->time_of_delay = 0;
    new_process->pmode = user_or_kernel;
    new_process->priority = priority;

    CALL( Z502MakeContext( &new_process->context, starting_address, user_or_kernel ) );
    CALL( InsertIntoList( PList, new_process ) );
    CALL( AddtoQueue( ready_queue, new_process, ORDER_PRIORITY ) );
    *p = new_process;
}


void DestoryProcess( PCB *p ){

    int LockResult;

    READ_MODIFY( MEMORY_READYQ_LOCK, DO_LOCK, SUSPEND_UNTIL_LOCKED,
                 &LockResult);
    if( RemoveFromQueue( ready_queue, p ) != 1 ){
        printf( "ERROR" );
        READ_MODIFY( MEMORY_READYQ_LOCK, DO_UNLOCK, SUSPEND_UNTIL_LOCKED,
                     &LockResult);
        return;
    }
        //RemoveFromQueue( timer_queue, p );

    READ_MODIFY( MEMORY_READYQ_LOCK, DO_UNLOCK, SUSPEND_UNTIL_LOCKED,
                 &LockResult);

    Z502DestroyContext( &p->context );
    DeleteByPid( PList, p->pid );

    free( p );
}
