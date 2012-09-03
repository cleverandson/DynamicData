//
//  Tests.h
//  DynamicData
//
//  Created by mich2 on 8/20/12.
//  Copyright (c) 2012 -. All rights reserved.
//

#ifndef DynamicData_Tests_h
#define DynamicData_Tests_h

#include "MMapWrapper.h"
#include "DDConfig.h"

class Tests
{
public:
    Tests(const Tests&) = delete;
    const Tests& operator=(const Tests&) = delete;

    
    static void mmapSpeedTest()
    {
        std::cout << "start " << std::endl;
        
        int fileHandle = open("testfile.data", O_RDWR | O_TRUNC | O_CREAT, (mode_t)0600);
        if (fileHandle == -1)
        {
            std::cout << "Tests: error opening file" << std::endl;
            exit(1);
        }
        
        unsigned long long size = 1000*1000*1000;
        ftruncate(fileHandle, size);
        
        char* map = (char*)mmap(0, size, PROT_READ | PROT_WRITE, MAP_SHARED, fileHandle, 0);
        
        std::cout << "11 " << std::endl;
        std::cout << "pp " << map[20] << std::endl;
        std::cout << "22 " << std::endl;
        
        if (munmap(map, size) == -1)
        {
            std::cout << "error " << std::endl;
        }
        
        std::cout << "done " << std::endl;
        
    }
    
    class UserDataHeader
    {
    public:
        int test;
    };
    
    
    static void testMMapWrapper()
    {
        std::cout << "start " << std::endl;
        
        MMapWrapper<ULongLong, ULongLong, UserDataHeader> wmap("testPath", 30);
        
        std::cout << "size__ " << wmap.size() << std::endl;
        std::cout << "filesize_s_ " << wmap.fileSize() << std::endl;
        
        
        for (int i=0; i<wmap.size(); i++)
        {
            std::cout << "elems__ " << wmap.getVal(i) << std::endl;
        }
        
        for (int i=0; i<10000; i++)
        {
            wmap.persistVal(i, i);
        }
        
        for (int i=0; i<9999; i++)
        {
            wmap.deleteIdx(0, [](ULongLong oldIdx, ULongLong remapIdx) {});
        }
        
        for (int i=0; i<10000; i++)
        {
            wmap.persistVal(i, i);
        }
        
        /*
        for (int i=0; i<9999; i++)
        {
            wmap.deleteIdx(0, [](ULongLong oldIdx, ULongLong remapIdx) {});
        }
        */
        
        wmap.shrink(9988);
        
        /*
        for (int i=0; i<10000; i++)
        {
            wmap.presistVal(i, i);
        }
        */
        
        std::cout << "filesize_e_ " << wmap.fileSize() << std::endl;
        
        std::cout << "done " << std::endl;
    }
    
    static void testDDMMapAllocator()
    {
        DDMMapAllocator<ULongLong>* mapAlloc = DDMMapAllocator<ULongLong>::SHARED();
        
        auto handle = mapAlloc->getHandleFromDataStore<ULongLong, UserDataHeader>(1, 0);
        //handle->presistVal(0, 12345);
    
        std::cout << "__ddd " << handle->getVal(0) << std::endl;
    }
    
    static void testDDContinuousID()
    {
        DDContinuousID<unsigned int> dd(1);
        DDContinuousID<unsigned int> dd2(32768);
        DDContinuousID<unsigned int> dd3(10000);
        DDContinuousID<unsigned int> dd4(65000);
        
        //std::cout << "__ddd " << dd.bound() << std::endl;
        
        bool check = dd2 < dd4;
        
        if (check)
        {
            std::cout << "TRUE " << std::endl;
        }
        else
        {
            std::cout << "FALSE " << std::endl;
        }
        
    }
    
    
    
};

#endif
