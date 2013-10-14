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

void DestoryMsg( int );
Message *CreateMsg( char*, int, int, int );

#endif // MESSAGE_H_INCLUDED
