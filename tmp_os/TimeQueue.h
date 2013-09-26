//
//  TimeQueue.h
//  Test0
//
//  Created by Wu Xin on 9/19/13.
//  Copyright (c) 2013 Wu Xin. All rights reserved.
//

#ifndef Test0_TimeQueue_h
#define Test0_TimeQueue_h
#include "PCB.h"

typedef struct
{
    void *head;
    ProcessBlock *nextPCB;
    
} TimeQueue;

void addToTimeQueue(ProcessBlock nextPCB);
#endif
