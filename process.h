#ifndef PROCESS_H_INCLUDED
#define PROCESS_H_INCLUDED

#include                 "z502.h"


struct PCB{
    int                 pid;
    int                 time_of_delay;
    char                name[64];
    INT16               state;
    INT16               pmode;
    INT16               priority;
    void                *context;
    struct PCB          *next_pcb;
};
typedef struct PCB PCB;

int                   global_pid;
PCB                   *running_process;

void OSCreateProcess( PCB **, void *, char *, int, BOOL );
void DestoryProcess( PCB* );

#endif // PROCESS_H_INCLUDED
