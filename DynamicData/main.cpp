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
#include "Tests.h"
#include "DDIndex.h"
#include "DDLoopReduce.h"
#include "DDSpawn.h"
#include "Demo.h"

#include <sys/stat.h>

int main(int argc, const char * argv[])
{
    //Tests::testAssertConfig();
    Tests::testBenchmarks();
 
    
    //Tests::testDDBaseVec();
    
    
    //Tests::testDDBaseSet();
    
    
    //old tests
    //Demo::demo();
    
    //Tests::mmapSpeedTest();
    //Tests::testMMapWrapper();
    //Tests::testDDMMapAllocator();
    //Tests::testDDContinuousID();
    
    //Tests::testDDIndex0(true);
    
    
    //Tests::testDDIndex(false);
    //Tests::testDDIndex(true);
    
    //Tests::testDDIndex2(false);
    //Tests::testDDIndex2(true);
    
    //Tests::testDDIndex3(false);
    //Tests::testDDIndex3(true);
    
    //TODO implement more test with updates!!
    //Tests::testWithUpdateDDIndex(false);
    
    
    //Tests::testTempTest(false);
    //Tests::testTempTest(true);
    
    //Tests::tester();
    
    //Tests::testFileSizes();
    
    //Tests::testDDLoopReduce();
    //Tests::testDDSpawn();
    
    //Tests::testDDDeleteField();
    //Tests::testDDInsertField();
    //Tests::testDDField();
    
    
    return 0;
}

