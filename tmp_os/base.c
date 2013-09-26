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

#include             "global.h"
#include             "syscalls.h"
#include             "protos.h"
#include             "string.h"
#include             "z502.h"
#include             "PCB.h"
#include "TimeQueue.h"


// These loacations are global and define information about the page table
extern UINT16        *Z502_PAGE_TBL_ADDR;
extern INT16         Z502_PAGE_TBL_LENGTH;

extern void          *TO_VECTOR [];

char                 *call_names[] = { "mem_read ", "mem_write",
                            "read_mod ", "get_time ", "sleep    ",
                            "get_pid  ", "create   ", "term_proc",
                            "suspend  ", "resume   ", "ch_prior ",
                            "send     ", "receive  ", "disk_read",
                            "disk_wrt ", "def_sh_ar" };

ProcessBlock         *wholePointer;
TimeQueue            timeQueue;


/************************************************************************
    INTERRUPT_HANDLER
        When the Z502 gets a hardware interrupt, it transfers control to
        this routine in the OS.
************************************************************************/
void    interrupt_handler( void ) {
    INT32              device_id;
    INT32              status;
    INT32              Index = 0;
    
    // Get cause of interrupt
    MEM_READ(Z502InterruptDevice, &device_id );
    // Set this device as target of our query
    MEM_WRITE(Z502InterruptDevice, &device_id );
    // Now read the status of this device
    MEM_READ(Z502InterruptStatus, &status );

    // Clear out this device - we're done with it
    MEM_WRITE(Z502InterruptClear, &Index );
    
    //take PCB off the timer queue
    timeQueue.head = NULL;
    printf("take of  the PCB on the timer queue successfully \n");
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
    // Clear out this device - we're done with it
    MEM_WRITE(Z502InterruptClear, &Index );
}                                       /* End of fault_handler */

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

#pragma mark - start_timer
void start_timer(SYSTEM_CALL_DATA *SystemCallData) {
    INT32 time;
    INT32 time_delay;
    INT32 Status;
    INT16 Z502_MODE;
    
    addToTimeQueue(*wholePointer);
//    timeQueue.head = wholePointer;
    printf("put PCB on the timeQueue successfully \n");
    
    GET_TIME_OF_DAY(&time);
    
    time_delay = (INT32)SystemCallData->Argument[0];
	MEM_WRITE(Z502TimerStart, &time_delay);
    MEM_READ(Z502TimerStatus, &Status);
    
	if (Status == DEVICE_IN_USE){
		printf("Got expected result for Status of Timer\n");
        Z502Idle();
    }
	else
		printf("Got erroneous result for Status of Timer\n");
}

void    svc( SYSTEM_CALL_DATA *SystemCallData ) {
    short               call_type;
    static short        do_print = 10;
//    short               i;
    INT32               Time=24;

    call_type = (short)SystemCallData->SystemCallNumber;
    if ( do_print > 0 ) {
        printf( "SVC handler: %s\n", call_names[call_type]);
//        for (i = 0; i < SystemCallData->NumberOfArguments - 1; i++ ){
//        	 Value = (long)*SystemCallData->Argument[i];
//             printf( "Arg %d: Contents = (Decimal) %8ld,  (Hex) %8lX\n", i,
//             (unsigned long )SystemCallData->Argument[i],
//             (unsigned long )SystemCallData->Argument[i]);
//        }
    do_print--;
        
        switch (call_type) {
            case SYSNUM_GET_TIME_OF_DAY:
                Z502MemoryRead(Z502ClockStatus, &Time);
                *SystemCallData->Argument[0] = Time;
                break;
                
            case SYSNUM_TERMINATE_PROCESS:
                Z502Halt();
                break;
                
            case SYSNUM_SLEEP:

                //svc hands control of the SLEEP request to start_timer, a routine YOU write.
                //start_timer, enqueues the PCB of the running process onto the timer_queue.
                
                start_timer(SystemCallData);
                break;
    
            default:
                printf("ERROR! call_type not recognized!\n");
                printf("Call_type is - %i\n",call_type);
        }
    }
}                                               // End of svc



/************************************************************************
    osInit
        This is the first routine called after the simulation begins.  This
        is equivalent to boot code.  All the initial OS components can be
        defined and initialized here.
************************************************************************/

#pragma mark - OSCreateProcess
void OSCreatProcess(void *identifier) {
    void                *next_context;
    
    Z502MakeContext(&next_context, (void *)identifier, USER_MODE);
    
    ProcessBlock pcb;
    wholePointer = &pcb;
    pcb.pointer = NULL;
    pcb.PID = 1;
    pcb.mode = 0; //0 == UserMode 1 == KernelMode
    pcb.proprity = 0;
    pcb.running_status = 1; //1 == running 0 == idle
    pcb.timeOfDelay = 100;
    pcb.context = next_context;
    
    Z502SwitchContext(SWITCH_CONTEXT_SAVE_MODE, &next_context);
}

void    osInit( int argc, char *argv[]  ) {
    void                *next_context;
    INT32               i;
    
    /* Demonstrates how calling arguments are passed thru to here       */

    printf( "Program called with %d arguments:", argc );
    for ( i = 0; i < argc; i++ )
        printf( " %s", argv[i] );
    printf( "\n" );
    printf( "Calling with argument 'sample' executes the sample program.\n" );

    /*          Setup so handlers will come to code in base.c           */

    TO_VECTOR[TO_VECTOR_INT_HANDLER_ADDR]   = (void *)interrupt_handler;
    TO_VECTOR[TO_VECTOR_FAULT_HANDLER_ADDR] = (void *)fault_handler;
    TO_VECTOR[TO_VECTOR_TRAP_HANDLER_ADDR]  = (void *)svc;

    /*  Determine if the switch was set, and if so go to demo routine.  */

    if (( argc > 1 ) && ( strcmp( argv[1], "sample" ) == 0 ) ) {
        Z502MakeContext( &next_context, (void *)sample_code, KERNEL_MODE );
        Z502SwitchContext( SWITCH_CONTEXT_KILL_MODE, &next_context );
    }                   /* This routine should never return!!           */

    /*  This should be done by a "os_make_process" routine, so that
        test0 runs on a process recognized by the operating system.    */
//    Z502MakeContext( &next_context, (void *)test1a, USER_MODE );
//    Z502SwitchContext( SWITCH_CONTEXT_KILL_MODE, &next_context );
    
    OSCreatProcess(test1a);
}                                               // End of osInit
