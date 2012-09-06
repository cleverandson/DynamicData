//
//  main.cpp
//  DynamicData
//
//  Created by mich mich on 8/15/12.
//  Copyright (c) 2012 -. All rights reserved.
//

#include <iostream>
#include "DDMMapAllocator.h"
#include "MMapWrapper.h"
#include "DDContinuousID.h"
#include "Tests.h"
#include "DDIndex.h"

#include <sys/stat.h>

int main(int argc, const char * argv[])
{
    //Tests::mmapSpeedTest();
    //Tests::testMMapWrapper();
    //Tests::testDDMMapAllocator();
    //Tests::testDDContinuousID();
    
    //Tests::testDDIndex(false);
    //Tests::testDDIndex(true);
    
    //Tests::testDDIndex2(false);
    //Tests::testDDIndex2(true);
    
    //Tests::testDDIndex3(false);
    //Tests::testDDIndex3(true);
    
    Tests::testFileSizes();
    
    return 0;
}

