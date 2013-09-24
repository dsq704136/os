#ifndef PROCESS_H_INCLUDED
#define PROCESS_H_INCLUDED

#include                 "z502.h"


typedef struct
    {
    INT32               pid;
    char                *name;
    struct PCB          *next_pcb;
    INT32               time_of_delay;
    INT16               running_status;
    INT16               pmode;
    INT16               priority;
    void                *context;
} PCB;

PCB                   *running_process;

PCB *OSCreateProcess( void *, char *, int, BOOL );

#endif // PROCESS_H_INCLUDED
