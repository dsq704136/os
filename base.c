/************************************************************************

        This code forms the base of the operating system you will
        build.  It has only the barest rudiments of what you will
        eventually construct; yet it contains the interfaces that
        allow test.c and z502.c to be successfully built together.

        Revision History:
        1.0 August 1990
        1.1 December 1990: Portability attempted.
        1.3 July     1992: More Portability enhancements.
                           Add call to sample_code.
        1.4 December 1992: Limit (temporarily) printout in
                           interrupt handler.  More portability.
        2.0 January  2000: A number of small changes.
        2.1 May      2001: Bug fixes and clear STAT_VECTOR
        2.2 July     2002: Make code appropriate for undergrads.
                           Default program start is in test0.
        3.0 August   2004: Modified to support memory mapped IO
        3.1 August   2004: hardware interrupt runs on separate thread
        3.11 August  2004: Support for OS level locking
	4.0  July    2013: Major portions rewritten to support multiple threads
************************************************************************/
#include             <stdio.h>
#include             <stdlib.h>

#include             "global.h"
#include             "syscalls.h"
#include             "protos.h"
#include             "string.h"
#include             "process.h"
#include             "queues.h"
#include             "linklist.h"
#include             "message.h"

// These loacations are global and define information about the page table
extern UINT16        *Z502_PAGE_TBL_ADDR;
extern INT16         Z502_PAGE_TBL_LENGTH;
UINT16               vaddr;

extern void          *TO_VECTOR [];

char                 *call_names[] = { "mem_read ", "mem_write",
                            "read_mod ", "get_time ", "sleep    ",
                            "get_pid  ", "create   ", "term_proc",
                            "suspend  ", "resume   ", "ch_prior ",
                            "send     ", "receive  ", "disk_read",
                            "disk_wrt ", "def_sh_ar" };


// Define the mapping relation between name and address of tests
char                 *test_name[]   = { "test0",  "test1a", "test1b", "test1c", "test1d",
                                        "test1e", "test1f", "test1g", "test1h", "test1i",
                                        "test1j", "test1k", "test1l", "test1m", "test2a",
                                        "test2b", "test2c" };
void                 *test_addr[]   = { test0,    test1a,   test1b,   test1c,   test1d,
                                        test1e,   test1f,   test1g,   test1h,   test1i,
                                        test1j,   test1k,   test1l,   test1m,   test2a,
                                        test2b,   test2c };
int                test_print_lvl[] = { 4,        4,        4,        2,        2,
                                        4,        2,        4,        2,        4,
                                        2,        4,        2,        2,        4,
                                        4,        4 };
int               interrupt_printer = 0;
int               scheduler_printer = 1;
int               memory_printer    = 1;

/*  Level Interrupt/svc/fault   Scheduler    Memory
      0           0(Limited)      0(No)        0(No)
      1           0               0            1(Yes)
      2           0               1(Yes)       0
      3           0               1            1
      4           1(Full)         0            0
      5           1               0            1
      6           1               1            0
      7           1               1            1      */


/************************************************************************
    INTERRUPT_HANDLER
        When the Z502 gets a hardware interrupt, it transfers control to
        this routine in the OS.
************************************************************************/
void    interrupt_handler( void ) {
    INT32              device_id;
    INT32              status;
    INT32              Index = 0;
    INT32              LockResult;

    // Get cause of interrupt
    MEM_READ(Z502InterruptDevice, &device_id );
    // Set this device as target of our query
    MEM_WRITE(Z502InterruptDevice, &device_id );
    // Now read the status of this device
    MEM_READ(Z502InterruptStatus, &status );

    if( interrupt_printer )
        printf( "Interrupt_handler: Found device ID %d with status %d\n",
                        device_id, status );

    READ_MODIFY( MEMORY_TIMERQ_LOCK, DO_LOCK, SUSPEND_UNTIL_LOCKED,
			     &LockResult );
    ReadytoGo();

    READ_MODIFY( MEMORY_TIMERQ_LOCK, DO_UNLOCK, SUSPEND_UNTIL_LOCKED,
			     &LockResult );

    // Clear out this device - we're done with it
    MEM_WRITE(Z502InterruptClear, &Index );

}                                       /* End of interrupt_handler */
/************************************************************************
    FAULT_HANDLER
        The beginning of the OS502.  Used to receive hardware faults.
************************************************************************/


void    fault_handler( void )
    {
    INT32       device_id;
    INT32       status;
    INT32       Index = 0;

    // Get cause of interrupt
    MEM_READ(Z502InterruptDevice, &device_id );
    // Set this device as target of our query
    MEM_WRITE(Z502InterruptDevice, &device_id );
    // Now read the status of this device
    MEM_READ(Z502InterruptStatus, &status );

    printf( "Fault_handler: Found vector type %d with value %d\n",
                        device_id, status );

    if( device_id == INVALID_MEMORY ){
        if( status >= VIRTUAL_MEM_PGS )
            Z502Halt();
            //Maybe there is a better way to do it

        if( Z502_PAGE_TBL_LENGTH == 0 ){
            Z502_PAGE_TBL_LENGTH = 1024;
            //TODO what is the maximum length of table
            Z502_PAGE_TBL_ADDR = (UINT16 *) calloc( sizeof(UINT16),
                                                    Z502_PAGE_TBL_LENGTH );
        }
        Z502_PAGE_TBL_ADDR[status] = vaddr++;
    }
    else
        Z502Halt();

    // Clear out this device - we're done with it
    MEM_WRITE(Z502InterruptClear, &Index );


}                                       /* End of fault_handler */

/************************************************************************
    check_input
        Check the legality of inputs for CreateProcess System Call.
************************************************************************/


int check_input( char *name, int priority ){

    if( priority < 0 || priority > 99 )
        return ERR_ILLEAGLE_PRIORITY;

    if( GetLength( PList ) > 20 )
        return ERR_OVER_CAPACITY;

    if( SearchByPname( PList, name ) )
        return ERR_DUPLICATE_PNAME;

    return ERR_SUCCESS;
}


/************************************************************************
    DestoryRunningandDispath
        Used for terminating the running process.
************************************************************************/


void DestoryRunningandDispath(){

    CALL( DeleteByPid( PList, running_process->pid ) );
    DispatchProcess();
}


/************************************************************************
    DispatchProcess
        Let the first process of ready queue to run. If both ready
        queue and timer queue are empty then call Z502Idle().
        If only ready queue is empty then wait until some process
        added into ready queue.
************************************************************************/

void DispatchProcess(){

    int LockResult;

    if( IsEmptyQueue( ready_queue ) ){

        if( IsEmptyQueue( timer_queue ) )
            Z502Halt();

        CALL( Z502Idle() );
        while( IsEmptyQueue( ready_queue ) ){
            CALL();
        }
    }

    READ_MODIFY( MEMORY_READYQ_LOCK, DO_LOCK, SUSPEND_UNTIL_LOCKED,
			     &LockResult );

    PCB *p;
    CALL( GetFirstPCB( &p, ready_queue ) );
    READ_MODIFY( MEMORY_READYQ_LOCK, DO_UNLOCK, SUSPEND_UNTIL_LOCKED,
                 &LockResult );

    SchedulerPrinter( "Dispatch", p->pid );

    READ_MODIFY( MEMORY_READYQ_LOCK, DO_LOCK, SUSPEND_UNTIL_LOCKED,
			     &LockResult );
    CALL( RemoveFromQueue( ready_queue, p ) );
    p->state = RUNNING;
    running_process = p;

    READ_MODIFY( MEMORY_READYQ_LOCK, DO_UNLOCK, SUSPEND_UNTIL_LOCKED,
                 &LockResult );

    CALL( Z502SwitchContext( SWITCH_CONTEXT_SAVE_MODE, &p->context ) );
}


/************************************************************************
    ReadytoGo
        Move a process from timer queue into ready queue, and reset timer
************************************************************************/

void ReadytoGo(){

    int    time;
    short  Z502_MODE;
    INT32  LockResult;
    PCB    *p = timer_queue->head;
    BOOL   TimerChanged = FALSE;

    while( 1 ){
        GET_TIME_OF_DAY(&time);
        if( p != NULL && time >= p->time_of_delay ){
            //SchedulerPrinter( "Ready2Go", p->pid );
            RemoveFromQueue( timer_queue, p );
            READ_MODIFY( MEMORY_READYQ_LOCK, DO_LOCK, SUSPEND_UNTIL_LOCKED,
			             &LockResult);
            AddtoQueue( ready_queue, p, ORDER_PRIORITY );
            READ_MODIFY( MEMORY_READYQ_LOCK, DO_UNLOCK, SUSPEND_UNTIL_LOCKED,
			             &LockResult);
            p = timer_queue->head;
            TimerChanged = TRUE;
        }
        else
            break;
    }

    if( TimerChanged )
        ResetTimer();

}


/************************************************************************
    StartTimer
        Put the sleep process into timer queue, reset timer and call
        Dispatcher.
************************************************************************/

void StartTimer( int delay ){

    int     time, LockResult;
    short   Z502_MODE;

    SchedulerPrinter( "Sleep", running_process->pid );

    READ_MODIFY( MEMORY_TIMERQ_LOCK, DO_LOCK, SUSPEND_UNTIL_LOCKED,
			     &LockResult );
    CALL( GET_TIME_OF_DAY(&time) );
    running_process->time_of_delay = time + delay;
    AddtoQueue( timer_queue, running_process, ORDER_TIME_OF_DELAY );
    running_process = NULL;

    ResetTimer();

    READ_MODIFY( MEMORY_TIMERQ_LOCK, DO_UNLOCK, SUSPEND_UNTIL_LOCKED,
			     &LockResult);
    DispatchProcess();
}


/************************************************************************
    ResetTimer
        Used to reset the timer( EventQ ) by the least delay in timer
        queue. Used whenever the timer queue change.
************************************************************************/

void ResetTimer(){

    PCB *p = NULL;
    int time, delay, Z502_MODE;

    GetFirstPCB( &p, timer_queue );
    if( p ){
        GET_TIME_OF_DAY(&time);
        delay = p->time_of_delay - time;
        if( delay > 0 )
            Z502MemoryWrite( Z502TimerStart, &delay );
        else{
            delay = 5;
            Z502MemoryWrite( Z502TimerStart, &delay );
        }
    }
}


/************************************************************************
    SchedulerPrinter

        Used to print the status of current system, including time,
        action, running process and content of timer queue, ready queue
        and suspend queue.
************************************************************************/


void SchedulerPrinter( char *act, int target ){

    if( !scheduler_printer )
        return;

    int    LockResult, time;
    short  Z502_MODE;
    PCB    *p;

    READ_MODIFY( MEMORY_PRINTER_LOCK, DO_LOCK, SUSPEND_UNTIL_LOCKED,
			     &LockResult);

    GET_TIME_OF_DAY( &time );
    SP_setup( SP_TIME_MODE, time );
	SP_setup_action( SP_ACTION_MODE, act );
	SP_setup( SP_TARGET_MODE, target );
    if( running_process )
        SP_setup( SP_RUNNING_MODE, running_process->pid );

    READ_MODIFY( MEMORY_READYQ_LOCK, DO_LOCK, SUSPEND_UNTIL_LOCKED,
			     &LockResult);
    p = ready_queue->head;
	while( p ){
		SP_setup( SP_READY_MODE, p->pid );
        p = p->next_pcb;
	}

    p = suspend_queue->head;
	while( p ){
		SP_setup( SP_SUSPENDED_MODE, p->pid );
        p = p->next_pcb;
	}

    READ_MODIFY( MEMORY_READYQ_LOCK, DO_UNLOCK, SUSPEND_UNTIL_LOCKED,
			     &LockResult);

    READ_MODIFY( MEMORY_TIMERQ_LOCK, DO_LOCK, SUSPEND_UNTIL_LOCKED,
			     &LockResult);
    p = timer_queue->head;
	while( p ){
		SP_setup( SP_WAITING_MODE, p->pid );
        p = p->next_pcb;
	}

    SP_print_header();
	SP_print_line();

    READ_MODIFY( MEMORY_TIMERQ_LOCK, DO_UNLOCK, SUSPEND_UNTIL_LOCKED,
			     &LockResult);

    READ_MODIFY( MEMORY_PRINTER_LOCK, DO_UNLOCK, SUSPEND_UNTIL_LOCKED,
			     &LockResult);

}


/************************************************************************
    SuspendProcess

************************************************************************/

void SuspendProcess( PCB *p, long *Err ){

    int LockResult;

    if( p->state == SUSPEND ){
        *Err = ERR_ALREADY_SUSPEND;
        return;
    }

    SchedulerPrinter( "SUSPEND", p->pid );
    READ_MODIFY( MEMORY_READYQ_LOCK, DO_LOCK, SUSPEND_UNTIL_LOCKED,
                 &LockResult );
    if( p->state == RUNNING ){
        CALL( AddtoQueue( suspend_queue, p, ORDER_OTHER ) );
        READ_MODIFY( MEMORY_READYQ_LOCK, DO_UNLOCK, SUSPEND_UNTIL_LOCKED,
                     &LockResult );
        DispatchProcess();
    }
    else if( p->state == WAITING ){
        CALL( RemoveFromQueue( timer_queue, p ) );
        CALL( AddtoQueue( suspend_queue, p, ORDER_OTHER ) );
        ResetTimer();
    }
    else{
        CALL( RemoveFromQueue( ready_queue, p ) );
        CALL( AddtoQueue( suspend_queue, p, ORDER_OTHER ) );
    }
    READ_MODIFY( MEMORY_READYQ_LOCK, DO_UNLOCK, SUSPEND_UNTIL_LOCKED,
                 &LockResult );
    *Err = ERR_SUCCESS;
}


/************************************************************************
    ResumeProcess

************************************************************************/

void ResumeProcess( PCB *p ){

    int LockResult;

    SchedulerPrinter( "RESUME", p->pid );
    READ_MODIFY( MEMORY_READYQ_LOCK, DO_LOCK, SUSPEND_UNTIL_LOCKED,
                 &LockResult );

    CALL( RemoveFromQueue( suspend_queue, p ) );
    CALL( AddtoQueue( ready_queue, p, ORDER_PRIORITY ) );

    READ_MODIFY( MEMORY_READYQ_LOCK, DO_UNLOCK, SUSPEND_UNTIL_LOCKED,
                 &LockResult );

}


/************************************************************************
    SVC
        The beginning of the OS502.  Used to receive software interrupts.
        All system calls come to this point in the code and are to be
        handled by the student written code here.
        The variable do_print is designed to print out the data for the
        incoming calls, but does so only for the first ten calls.  This
        allows the user to see what's happening, but doesn't overwhelm
        with the amount of data.
************************************************************************/

void    svc( SYSTEM_CALL_DATA *SystemCallData ) {
    short               call_type, i;
    static short        do_print = 10;
    void                *addr;
    char                *name, *msg, *msg_buf;
    int                 Time, priority, r, pid,
                        LockResult, buf_size;
    LinkList            n = NULL;
    PCB                 *p;

    call_type = (short)SystemCallData->SystemCallNumber;
    if ( do_print > 0 ) {
        printf( "SVC handler: %s\n", call_names[call_type]);
        if( interrupt_printer )
            for (i = 0; i < SystemCallData->NumberOfArguments - 1; i++ ){
                printf( "Arg %d: Contents = (Decimal) %8ld,  (Hex) %8lX\n", i,
                (unsigned long )SystemCallData->Argument[i],
                (unsigned long )SystemCallData->Argument[i]);
            }
        do_print--;
    }
    switch (call_type) {
        // Get time service call
        case SYSNUM_GET_TIME_OF_DAY:   // This value is found in syscalls.h
            CALL( Z502MemoryRead( Z502ClockStatus, &Time ) );
            *(int *)SystemCallData->Argument[0] = Time;
            break;
        // terminate system call
        case SYSNUM_TERMINATE_PROCESS:

            pid = (int)SystemCallData->Argument[0];
            if( pid == -2 )
                Z502Halt();

            if( pid == -1 ){
                CALL( DestoryRunningandDispath() );
            }
            else{
                CALL( n = SearchByPid( PList, pid ) );
                if( n == NULL ){
                    *(long *)SystemCallData->Argument[1] = ERR_NO_SUCH_PROCESS;
                    break;
                }
                p = n->data;
                if( p->state == RUNNING )
                    CALL( DestoryRunningandDispath() );

                CALL( DestoryProcess( p ) );
                *(long *)SystemCallData->Argument[1] = ERR_SUCCESS;
            }
            break;

        case SYSNUM_SLEEP:
            CALL( StartTimer( (int)SystemCallData->Argument[0] ) );
            break;

        case SYSNUM_REVOKE:
            pid = (int)SystemCallData->Argument[0];
            n = SearchByPid( PList, pid );
            if( !n ){
                *(long *)SystemCallData->Argument[1] = ERR_PID_NOT_FOUND;
                break;
            }
            p = n->data;
            if( p->state != WAITING ){
                *(long *)SystemCallData->Argument[1] = ERR_NOT_SLEEP;
                break;
            }
            SchedulerPrinter( "REVOKE", p->pid );
            READ_MODIFY( MEMORY_TIMERQ_LOCK, DO_LOCK, SUSPEND_UNTIL_LOCKED,
                         &LockResult );

            CALL( RemoveFromQueue( timer_queue, p ) );

            READ_MODIFY( MEMORY_TIMERQ_LOCK, DO_UNLOCK, SUSPEND_UNTIL_LOCKED,
                         &LockResult );

            READ_MODIFY( MEMORY_READYQ_LOCK, DO_LOCK, SUSPEND_UNTIL_LOCKED,
                         &LockResult );

            CALL( AddtoQueue( ready_queue, p, ORDER_PRIORITY ) );

            READ_MODIFY( MEMORY_READYQ_LOCK, DO_UNLOCK, SUSPEND_UNTIL_LOCKED,
                         &LockResult );
            ResetTimer();

            *(long *)SystemCallData->Argument[1] = ERR_SUCCESS;
            break;


        case SYSNUM_CREATE_PROCESS:
            name = (char *)SystemCallData->Argument[0];
            addr = SystemCallData->Argument[1];
            priority = (int)SystemCallData->Argument[2];

            CALL( r = check_input( name, priority ) );
            if( r == ERR_SUCCESS ){
                CALL( OSCreateProcess( &p, addr, name, priority, USER_MODE ) );
                *(int *)SystemCallData->Argument[3] = p->pid;
                SchedulerPrinter( "Created", p->pid );
            }

            *(long *)SystemCallData->Argument[4] = r;
            break;

        case SYSNUM_GET_PROCESS_ID:
            name = (char *)SystemCallData->Argument[0];
            if( strcmp( name, "" ) == 0 )
                p = running_process;

            else{
                CALL( n = SearchByPname( PList, name ) );
                if( n != NULL )
                    p = n->data;
                else{
                    *(long *)SystemCallData->Argument[2] = ERR_PID_NOT_FOUND;
                    break;
                }
            }
            *(int *)SystemCallData->Argument[1] = p->pid;
            *(long *)SystemCallData->Argument[2] = ERR_SUCCESS;
            break;

        case SYSNUM_SUSPEND_PROCESS:
            pid = (int)SystemCallData->Argument[0];
            CALL( n = SearchByPid( PList, pid ) );
            if( n ){
                p = n->data;
                SuspendProcess( p, SystemCallData->Argument[1] );
            }
            else
                *(long *)SystemCallData->Argument[1] = ERR_PID_NOT_FOUND;

            break;

        case SYSNUM_RESUME_PROCESS:

            pid = (int)SystemCallData->Argument[0];
            CALL( n = SearchByPid( PList, pid ) );
            if( !n ){
                //Illegal PID
                *(long *)SystemCallData->Argument[1] = ERR_PID_NOT_FOUND;
                break;
            }

            p = n->data;
            if( p->state != SUSPEND ){
                *(long *)SystemCallData->Argument[1] = ERR_NOT_SUSPEND;
                break;
            }
            ResumeProcess( p );
            *(long*)SystemCallData->Argument[1] = ERR_SUCCESS;
            break;

        case SYSNUM_CHANGE_PRIORITY:

            pid = (int)SystemCallData->Argument[0];
            priority = (int)SystemCallData->Argument[1];
            if( priority < 0 || priority > 99 ){
                //Illegal Priority
                *(long*)SystemCallData->Argument[2] = ERR_ILLEAGLE_PRIORITY;
                break;
            }
            if( pid == -1 ){
                running_process->priority = priority;
            }
            else{
                CALL( n = SearchByPid( PList, pid ) );
                if( !n ){
                    *(long*)SystemCallData->Argument[2] = ERR_PID_NOT_FOUND;
                    break;
                }

                p = n->data;
                SchedulerPrinter("ChPrior",p->pid);
                p->priority = priority;

                if( p->state == READY ){
                    //Order again after priority changed
                    READ_MODIFY( MEMORY_READYQ_LOCK, DO_LOCK, SUSPEND_UNTIL_LOCKED,
                                 &LockResult );

                    CALL( RemoveFromQueue( ready_queue, p ) );
                    CALL( AddtoQueue( ready_queue, p, ORDER_PRIORITY ) );

                    READ_MODIFY( MEMORY_READYQ_LOCK, DO_UNLOCK, SUSPEND_UNTIL_LOCKED,
                                 &LockResult );
                }
            }

            *(long*)SystemCallData->Argument[2] = ERR_SUCCESS;
            break;

        case SYSNUM_SEND_MESSAGE:

            pid = (int)SystemCallData->Argument[0];
            msg = (char*)SystemCallData->Argument[1];
            buf_size = (int)SystemCallData->Argument[2];

            if( buf_size <= 0 || buf_size > 128 || buf_size < strlen( msg ) ){
                *(long*)SystemCallData->Argument[3] = ERR_ILLEAGLE_BUFFER;
                break;
            }
            if( pid != -1 ){
                CALL( n = SearchByPid( PList, pid ) );
                if( !n ){
                    *(long*)SystemCallData->Argument[3] = ERR_PID_NOT_FOUND;
                    break;
                }
            }

            if( GetLength( MsgList ) > 20 ){
                *(long*)SystemCallData->Argument[3] = ERR_OVER_CAPACITY;
                break;
            }

            Message *m = CreateMsg( msg, buf_size, running_process->pid, pid );
            InsertIntoList( MsgList, m );

            /* Resume process which is suspended for message */
            if( n ){
                p = n->data;
                if( HasPCB( suspend_queue, p ) )
                    ResumeProcess( p );
            }
            else{
                /* Broadcast message, resume a process
                   who are suspended for a message.     */
                CALL( GetFirstPCB( &p, suspend_queue ) );
                if( p )
                    ResumeProcess( p );
            }

            *(long*)SystemCallData->Argument[3] = ERR_SUCCESS;
            break;

        case SYSNUM_RECEIVE_MESSAGE:

            pid = (int)SystemCallData->Argument[0];
            msg_buf = (char*)SystemCallData->Argument[1];
            buf_size = (int)SystemCallData->Argument[2];

            if( buf_size < 0 || buf_size > 128 ){
                *(long*)SystemCallData->Argument[5] = ERR_ILLEAGLE_BUFFER;
                break;
            }

            if( pid != -1 ){
                // Not a broadcast message
                CALL( n = SearchByPid( PList, pid ) );
                if( !n ){
                    *(long*)SystemCallData->Argument[5] = ERR_PID_NOT_FOUND;
                    break;
                }
            }

            Message *msg;
            // Suspend if no message can be received
            while( !( msg = GetMsg( pid, running_process->pid ) ) )
                SuspendProcess( running_process, SystemCallData->Argument[5] );

            if( buf_size < msg->length ){
                *(long*)SystemCallData->Argument[5] = ERR_ILLEAGLE_BUFFER;
                break;
            }
            *(int*)SystemCallData->Argument[3] = msg->length;
            *(int*)SystemCallData->Argument[4] = msg->sender_pid;
            strcpy( msg_buf, msg->data );
            DeleteMsg( msg->id );

            *(long*)SystemCallData->Argument[5] = ERR_SUCCESS;
            break;


        default:
            printf( "ERROR!  call_type not recognized!\n" );
            printf( "Call_type is - %i\n", call_type);
    }                                           // End of switch
}                                               // End of svc



/************************************************************************
    GetTestAddr
        Used for find the address of test function by test name.
        I cannot find a way to get the addresses list from test.c by name,
        so I use this easy but inflexible way to achieve the function.
        The mapping relation between name and address defined above.
************************************************************************/

void *GetTestAddr( char* name ){

    int i = 0;
    while( test_name[i] ){
        if( strcmp( name, test_name[i] ) == 0 )
            return test_addr[i];
        i++;
    }
    return NULL;
}


/************************************************************************
    GetTestPrintLevel
        Used for find the printer level of test function by test name.
        Level is a integer ranged from 0~7, which is 000 ~ 111 in Binary.
        Each bit represents the level of certain printer, such as scheduler
        or memory. the detail of level shows on the top of this file.
************************************************************************/

int GetTestPrintLevel( char* name ){

    int i = 0;
    while( test_name[i] ){
        if( strcmp( name, test_name[i] ) == 0 )
            return test_print_lvl[i];
        i++;
    }
    return -1;
}


/************************************************************************
    ListTest
        List all test name we have.
************************************************************************/

void ListTest(){

    int i = 1;
    printf( "\n======================== Test List ========================\n" );
    while( test_name[i-1] ){
        printf( "%10s,", test_name[i-1] );
        if( i%5 == 0 )
            printf("\n");
        i++;
    }
    printf( "\n=========================== End ===========================\n" );
    printf("\n");

}


/************************************************************************
    osInit
        This is the first routine called after the simulation begins.  This
        is equivalent to boot code.  All the initial OS components can be
        defined and initialized here.
************************************************************************/

void    osInit( int argc, char *argv[] ) {
    INT32               i;
    /* Demonstrates how calling arguments are passed thru to here       */

    /* Initial all system structures */
    CALL( InitQueue( &timer_queue, "Timer" ) );
    CALL( InitQueue( &ready_queue, "Ready" ) );
    CALL( InitQueue( &suspend_queue, "Suspend" ) );
    CALL( PList = CreateNullList() );
    CALL( MsgList = CreateNullList() );
    running_process = NULL;
    global_pid = 0;
    global_msg_id = 0;
    vaddr = PTBL_VALID_BIT;
    PCB *p;

    printf( "Program called with %d arguments:", argc );
    for ( i = 0; i < argc; i++ )
        printf( " %s", argv[i] );
    printf( "\n" );
    //printf( "Calling with argument 'sample' executes the sample program.\n" );

    /*          Setup so handlers will come to code in base.c           */

    TO_VECTOR[TO_VECTOR_INT_HANDLER_ADDR]   = (void *)interrupt_handler;
    TO_VECTOR[TO_VECTOR_FAULT_HANDLER_ADDR] = (void *)fault_handler;
    TO_VECTOR[TO_VECTOR_TRAP_HANDLER_ADDR]  = (void *)svc;

    /*  Determine if the switch was set, and if so go to demo routine.  */

    if (( argc > 1 ) && ( strcmp( argv[1], "sample" ) == 0 ) )
        CALL( OSCreateProcess( &p,(void *)sample_code, "", 0, KERNEL_MODE ) );
                       /* This routine should never return!!           */

    /*  This should be done by a "os_make_process" routine, so that
        test0 runs on a process recognized by the operating system.    */
    if( argc < 2 ){
        printf( "\nERROR: Need a argument for assigning test\n");
        printf( "         Example: bash$ ./OS502 test1c\n" );
        return;
    }

    char *t_name = argv[1];
    void *Test;

    /* Get test address from name we entered. */
    while( !( Test = GetTestAddr( t_name ) ) ){
        ListTest();
        printf( "Test( %s ) not found, please enter the test again\n", t_name );
        printf( "from tests list above, or enter 'exit': " );
        scanf( "%s", t_name );
        if( strcmp( t_name, "exit" ) == 0 )
            exit(0);
    }

    /* Get test printer level from name we entered. */
    int printer_lvl = GetTestPrintLevel( t_name );
    interrupt_printer = printer_lvl / 4;
    scheduler_printer = ( printer_lvl % 4 ) / 2;
    memory_printer = printer_lvl % 2;

    CALL( OSCreateProcess( &p, Test, argv[1], 0, USER_MODE ) );
    while(1){
        DispatchProcess();
    }

}                                               // End of osInit
