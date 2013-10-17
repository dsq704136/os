/*********************************************************************

    message.h

    Author : Shuoqing Ding
    Date   : 2013/10/14

    Define Message structure and some related global variables.
    Message structure is used for communication between processes.
    Store in LinkList Structure.

*********************************************************************/


#ifndef MESSAGE_H_INCLUDED
#define MESSAGE_H_INCLUDED

typedef struct Message{

    int   id;
    char  data[256];
    int   length;
    int   sender_pid;
    int   receiver_pid;

}Message;

int global_msg_id;

Message *CreateMsg( char*, int, int, int );

#endif // MESSAGE_H_INCLUDED
