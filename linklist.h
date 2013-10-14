#ifndef LINKLIST_H_INCLUDED
#define LINKLIST_H_INCLUDED

#include                 "process.h"
#include                 "message.h"


typedef struct LNode{

    void  *data;
    struct LNode *next;

}LNode, *LinkList;

LinkList              PList;
LinkList              MsgList;

LinkList CreateNullList( void );
int InsertIntoList( LinkList, void* );
Message *SearchMsg( int, int );
LinkList SearchByPid( LinkList, int );
LinkList SearchByPname( LinkList, char* );
BOOL DeleteByPid( LinkList, int );
BOOL DeleteByPname( LinkList, char* );
int GetLength( LinkList );

#endif // LINKLIST_H_INCLUDED
