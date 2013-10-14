#include "message.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

Message *CreateMsg( char *data, int len, int sender, int receiver ){

    Message *msg = ( Message* )calloc( 1, sizeof( Message ) );

    memset( msg->data, 0x00, 128 );
    strcpy( msg->data, data );
    msg->length = len;
    msg->id = ++global_msg_id;
    msg->sender_pid = sender;
    msg->receiver_pid = receiver;

    return msg;
}

