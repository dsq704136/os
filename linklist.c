#include                 <string.h>
#include                 <stdio.h>
#include                 <stdlib.h>
#include                 "global.h"
#include                 "syscalls.h"
#include                 "process.h"
#include                 "protos.h"
#include                 "queues.h"
#include                 "message.h"

#include "linklist.h"

LinkList CreateNullList( void ){

    LinkList L = (LinkList)calloc(1, sizeof(LNode));
    if( L == NULL ){
        printf( "Out of memory!" );
        return(NULL);
    }

    L->next = NULL;
    return(L);
}


int InsertIntoList( LinkList L, void *data ){

    LinkList new_node = (LinkList)calloc(1, sizeof(LNode));
    if( new_node == NULL ){
        printf( "Out of memory!" );
        return 0;
    }
    new_node->data = data;
    new_node->next = L->next;
    L->next = new_node;
    return 1;
}


Message *GetMsg( int sender, int receiver ){

    LinkList n = MsgList->next;
    Message *m;

    while( n ){
        m = n->data;
        if( ( sender == -1 || sender == m->sender_pid )
            && ( receiver == -1 || receiver == m->receiver_pid || m->receiver_pid == -1 ) )
            return m;

        n = n->next;
    }
    return NULL;
}



LinkList SearchByPid( LinkList L, int pid ){

    LinkList n = L->next;
    PCB *p;

    while( n ){

        p = n->data;

        if( p->pid == pid ){
            return n;
        }
        n = n->next;
    }
    return NULL;
}

LinkList SearchByPname( LinkList L, char *pname ){

    LinkList n = L->next;
    PCB *p;

    while( n ){

        p = n->data;
        if( ! strcmp( p->name, pname ) )
            return n;

        n = n->next;
    }
    return NULL;
}


BOOL DeleteMsg( int msg_id ){

    LinkList pre, n = MsgList;
    Message *m;

    while( n ){
        pre = n;
        n = n->next;
        m = n->data;
        if( m->id == msg_id ){
            pre->next = n->next;
            free( n );
            return TRUE;
        }
    }
    return FALSE;
}


BOOL DeleteByPid( LinkList L, int pid ){

    LinkList n, pre;
    n = L;
    PCB *p;

    while( n ){

        pre = n;
        n = n->next;
        p = n->data;

        if( p->pid == pid ){
            pre->next = n->next;
            free(n);
            return 1;
        }
    }
    return 0;
}

BOOL DeleteByPname( LinkList L, char *pname ){

    LinkList n, pre;
    n = L;
    PCB *p;

    while( n ){

        pre = n;
        n = n->next;
        p = n->data;

        if( ! strcmp( p->name, pname ) ){
            pre->next = n->next;
            free(n);
            return 1;
        }
    }
    return 0;
}

int GetLength( LinkList L ){

    LinkList n = L;
    int length = 0;

    while( n != NULL ){
        n = n->next;
        length++;
    }

    return length;
}
