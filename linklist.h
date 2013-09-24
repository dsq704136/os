#ifndef LINKLIST_H_INCLUDED
#define LINKLIST_H_INCLUDED

#include "process.h"


typedef struct LNode{

    PCB  *data;
    struct LNode *next;

}LNode, *LinkList;

LinkList              PList;

LinkList CreateNullList( void );
int InsertIntoList( LinkList, PCB* );
LinkList SearchByPid( LinkList, INT32 );
LinkList SearchByPname( LinkList, char* );
BOOL DeleteByPid( LinkList, INT32 );
BOOL DeleteByPname( LinkList, char* );
int GetLength( LinkList );

#endif // LINKLIST_H_INCLUDED
