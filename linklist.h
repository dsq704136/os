/*********************************************************************

    linklist.h

    Author : Shuoqing Ding
    Date   : 2013/10/14

    Define LNode structure and some related global variables.
    LNode structure is a standard LinkList, I use it to store all
    PCBs( PList ) and Messages( MsgList ).

*********************************************************************/

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
Message *GetMsg( int, int );
BOOL DeleteMsg( int );
LinkList SearchByPid( LinkList, int );
LinkList SearchByPname( LinkList, char* );
BOOL DeleteByPid( LinkList, int );
BOOL DeleteByPname( LinkList, char* );
int GetLength( LinkList );

#endif // LINKLIST_H_INCLUDED
