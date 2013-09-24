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



PCB *OSCreateProcess( void *starting_address, char *name,
                      int priority, BOOL user_or_kernel){

    PCB *new_process = (PCB *) calloc(1, sizeof(PCB));

    srand((unsigned int)time(NULL));
    new_process->name = name;
    new_process->pid = rand()%10000;
    new_process->next_pcb = NULL;
    new_process->time_of_delay = 0;
    new_process->pmode = user_or_kernel;
    new_process->priority = priority;
    new_process->state = WAITING;

    Z502MakeContext( &new_process->context, starting_address, user_or_kernel );
    InsertIntoList( PList, new_process );
    AddtoQueue( ready_queue, new_process, 'priority' );
    //DispatchProcess();
    //running_process = new_process;
    //Z502SwitchContext( SWITCH_CONTEXT_KILL_MODE, &new_process->context );

}

void SwitchProcess( PCB *p ){
    if( running_process ){
        AddtoQueue( ready_queue, running_process, "" );
    }
    RemoveFromQueue( ready_queue, p );
    running_process = p;
    Z502SwitchContext( SWITCH_CONTEXT_SAVE_MODE, &p->context );
}
