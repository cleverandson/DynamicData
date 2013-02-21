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
    //runs tests on the different benchmarks and checks the inserted
    //values for consistency.
    //Tests::testAssertConfig();
    
    //runs the different benchmarks in random order and prints out the
    //results
    Tests::testBenchmarks();
 
    return 0;
}

