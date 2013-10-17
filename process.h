/*********************************************************************

    process.h

    Author : Shuoqing Ding
    Date   : 2013/10/14

    Define PCB structure and some related global variables.
    A PCB is a process, it contains the necessary attributes of process.

*********************************************************************/


#ifndef PROCESS_H_INCLUDED
#define PROCESS_H_INCLUDED

#include                 "z502.h"


struct PCB{
    int                 pid;
    int                 time_of_delay;  //Time to wake up
    char                name[64];
    int                 state;          //Running, Ready, Waiting
    int                 pmode;          //UserMode or KernelMode
    int                 priority;
    void                *context;
    struct PCB          *next_pcb;
};
typedef struct PCB PCB;

int                   global_pid;
PCB                   *running_process;

void OSCreateProcess( PCB **, void *, char *, int, BOOL );
void DestoryProcess( PCB* );

#endif // PROCESS_H_INCLUDED
