//
//  PCB.h
//  Test0
//
//  Created by Wu Xin on 9/19/13.
//  Copyright (c) 2013 Wu Xin. All rights reserved.
//

#ifndef Test0_PCB_h
#define Test0_PCB_h
typedef struct
{
    int PID;
    int running_status;
    int mode;
    int proprity;
    float timeOfDelay;
    void *pointer;
    void *context;
} ProcessBlock;


#endif
