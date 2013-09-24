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
#include             "process.h"
#include             "queues.h"
#include             "linklist.h"

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



/************************************************************************
    INTERRUPT_HANDLER
        When the Z502 gets a hardware interrupt, it transfers control to
        this routine in the OS.
************************************************************************/
void    interrupt_handler( void ) {
    INT32              device_id;
    INT32              status;
    INT32              Index = 0;
    //static INT32       how_many_interrupt_entries = 0;    /** TEMP **/

    // Get cause of interrupt
    MEM_READ(Z502InterruptDevice, &device_id );
    // Set this device as target of our query
    MEM_WRITE(Z502InterruptDevice, &device_id );
    // Now read the status of this device
    MEM_READ(Z502InterruptStatus, &status );

    /** REMOVE THE NEXT SIX LINES **/
    /**how_many_interrupt_entries++;
    if ( how_many_interrupt_entries < 20 ){
        printf( "Interrupt_handler: Found device ID %d with status %d\n",
                        device_id, status );
    }**/

    // Clear out this device - we're done with it
    MEM_WRITE(Z502InterruptClear, &Index );
    RemoveFromQueue( timer_queue, running_process );
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

int check_input( char *name, INT32 proprity ){

    if( proprity < 0 )
        return ERR_ILLEAGLE_PRIORITY;

    if( GetLength( PList ) >= 20 )
        return ERR_OVER_CAPACITY;

    if( SearchByPname( PList, name ) )
        return ERR_DUPLICATE_PNAME;

    return ERR_SUCCESS;
}
/************************************************************************
    DispatchProcess
************************************************************************/

void DispatchProcess(){

    PCB *p;
    while( 1 ){
        p= ready_queue->head;

        if( p == NULL ){
            printf( "No Process." );
        }
        else{
            SwitchProcess( p );
        }
    }
}


/************************************************************************
    start_timer
************************************************************************/

void start_timer( INT32 delay ){

    INT32 timer_status;
    static INT32 time;
    INT16 Z502_MODE;

    GET_TIME_OF_DAY(&time);
    running_process->time_of_delay = time + delay;

    AddtoQueue( timer_queue, running_process, "time_of_delay" );

    //TODO CHECK STATUS FIRST
    /*Z502MemoryRead(Z502TimerStatus, &timer_status);
    if (timer_status != DEVICE_FREE){
        printf("Got erroneous result for Status of Timer\n");
        return;
    }*/

    Z502MemoryWrite(Z502TimerStart, &delay);
    Z502MemoryRead(Z502TimerStatus, &timer_status);

    if (timer_status != DEVICE_IN_USE){
        printf("Got erroneous result for Status of Timer\n");
        return;
    }

    Z502Idle();
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
    short               call_type;
    static short        do_print = 10;
    short               i;
    long                Time;
    void                *addr;
    char                *name;
    int                 prop;
    int                 r;
    LinkList            n;

    call_type = (short)SystemCallData->SystemCallNumber;
    if ( do_print > 0 ) {
        printf( "SVC handler: %s\n", call_names[call_type]);
        for (i = 0; i < SystemCallData->NumberOfArguments - 1; i++ ){
        	 //Value = (long)*SystemCallData->Argument[i];
             printf( "Arg %d: Contents = (Decimal) %8ld,  (Hex) %8lX\n", i,
             (unsigned long )SystemCallData->Argument[i],
             (unsigned long )SystemCallData->Argument[i]);
        }
        do_print--;
    }
    switch (call_type) {
        // Get time service call
        case SYSNUM_GET_TIME_OF_DAY:   // This value is found in syscalls.h
            Z502MemoryRead( Z502ClockStatus, &Time );
            *(long *)SystemCallData->Argument[0] = Time;
            break;
        // terminate system call
        case SYSNUM_TERMINATE_PROCESS:
            Z502Halt();
            break;

        case SYSNUM_SLEEP:
            start_timer( SystemCallData->Argument[0] );
            break;

        case SYSNUM_CREATE_PROCESS:
            name = SystemCallData->Argument[0];
            addr = SystemCallData->Argument[1];
            prop = SystemCallData->Argument[2];

            if( ( r = check_input( name, prop ) ) == ERR_SUCCESS )
                SystemCallData->Argument[3] = OSCreateProcess( addr, name, prop, USER_MODE );

            *(long *)SystemCallData->Argument[4] = r;
            break;

        case SYSNUM_GET_PROCESS_ID:
            name = SystemCallData->Argument[0];

            if( n = SearchByPname( PList, name ) ){
                SystemCallData->Argument[1] = n->data;
                SystemCallData->Argument[2] = ERR_SUCCESS;
            }
            else{
                SystemCallData->Argument[2] = ERR_PID_NOT_FOUND;
            }
            break;

        default:
            printf( "ERROR!  call_type not recognized!\n" );
            printf( "Call_type is - %i\n", call_type);
    }                                           // End of switch
}                                               // End of svc


/************************************************************************
    osInit
        This is the first routine called after the simulation begins.  This
        is equivalent to boot code.  All the initial OS components can be
        defined and initialized here.
************************************************************************/

void    osInit( int argc, char *argv[]  ) {
    INT32               i;
    /* Demonstrates how calling arguments are passed thru to here       */
    InitQueue( &timer_queue );
    InitQueue( &ready_queue );
    PList = CreateNullList();
    running_process = NULL;

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

    if (( argc > 1 ) && ( strcmp( argv[1], "sample" ) == 0 ) )
        OSCreateProcess( (void *)sample_code, "", 0, KERNEL_MODE );
                       /* This routine should never return!!           */

    /*  This should be done by a "os_make_process" routine, so that
        test0 runs on a process recognized by the operating system.    */

    OSCreateProcess( (void *)test1b, "", 0, USER_MODE );
    DispatchProcess();
    //Z502MakeContext( &next_context, (void *)test1a, USER_MODE );
    //Z502SwitchContext( SWITCH_CONTEXT_KILL_MODE, &next_context );
}                                               // End of osInit
