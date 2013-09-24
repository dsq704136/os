#include                 <string.h>
#include                 <stdio.h>
#include                 <stdlib.h>
#include                 <time.h>
#include                 "global.h"
#include                 "syscalls.h"
#include                 "process.h"
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

    Z502MakeContext( &new_process->context, starting_address, user_or_kernel );
    running_process = new_process;
    Z502SwitchContext( SWITCH_CONTEXT_KILL_MODE, &new_process->context );
    InsertIntoList( PList, new_process );
}
