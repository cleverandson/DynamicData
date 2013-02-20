/*
 
    This file is part of DynamicData.

    DynamicData is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Foobar is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Foobar.  If not, see <http://www.gnu.org/licenses/>.
 
*/

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

