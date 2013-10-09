#include                 <string.h>
#include                 <stdio.h>
#include                 <stdlib.h>
#include                 "global.h"
#include                 "syscalls.h"
#include                 "process.h"
#include                 "protos.h"
#include                 "queues.h"

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


int InsertIntoList( LinkList L, PCB *p ){

    LinkList new_node = (LinkList)calloc(1, sizeof(LNode));
    if( new_node == NULL ){
        printf( "Out of memory!" );
        return 0;
    }
    new_node->data = p;
    new_node->next = L->next;
    L->next = new_node;
    return 1;


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
