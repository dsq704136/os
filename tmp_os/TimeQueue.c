//
//  TimeQueue.c
//  Test0
//
//  Created by Wu Xin on 9/19/13.
//  Copyright (c) 2013 Wu Xin. All rights reserved.
//

#include <stdio.h>
#include "TimeQueue.h"

void addToTimeQueue(ProcessBlock nextPCB) {
    TimeQueue time;
    time.head = &nextPCB;
}