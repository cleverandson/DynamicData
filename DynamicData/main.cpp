//
//  main.cpp
//  DynamicData
//
//  Created by mich mich on 8/15/12.
//  Copyright (c) 2012 -. All rights reserved.
//

#include <iostream>
#include "DDFunctStack.h"
#include "DDMMapAllocator.h"
#include "MMapWrapper.h"
#include "DDClosureAllocator.h"
#include "DDClosureStack.h"
#include "DDContinuousID.h"
#include "Tests.h"
#include "DDIndex.h"

#include <sys/stat.h>

int main(int argc, const char * argv[])
{
    //Tests::mmapSpeedTest();
    Tests::testMMapWrapper();
    //Tests::testDDMMapAllocator();
    //Tests::testDDContinuousID();
    
    return 0;
}

