#ifndef PROCESS_H_INCLUDED
#define PROCESS_H_INCLUDED

#include                 "z502.h"


typedef struct
    {
    int                 pid;
    char                name[64];
    struct PCB          *next_pcb;
    int                 time_of_delay;
    INT16               state;
    INT16               pmode;
    INT16               priority;
    void                *context;
} PCB;

int                   global_pid;
PCB                   *running_process;

PCB *OSCreateProcess( void *, char *, int, BOOL );
void SwitchProcess( PCB* );
void DestoryProcess( PCB* );

#endif // PROCESS_H_INCLUDED
