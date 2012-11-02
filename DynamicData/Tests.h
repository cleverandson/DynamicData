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
#include "DDIndex.h"
#include "DDLoopReduce.h"
#include "DDSpawn.h"
#include "MMapWrapper.h"
#include "DDActivePassivePtr.h"
#include "DDInsertField.h"
#include "DDDeleteField.h"
#include "DDField.h"

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
        
        MMapWrapper<unsigned long long, unsigned long long, UserDataHeader> wmap("testPath", 30);
        
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
            wmap.deleteIdx(0, [](unsigned long long oldIdx, unsigned long long remapIdx) {});
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
        
        //wmap.shrinkSize(9);
        
        /*
        for (int i=0; i<10000; i++)
        {
            wmap.presistVal(i, i);
        }
        */
        
        std::cout << "filesize_e_ " << wmap.fileSize() << std::endl;
        
        std::cout << "done " << std::endl;
    }
    
    static void testFileSizes()
    {
        system("rm -r data");
        
        DDIndex<unsigned long, unsigned long> ddIndex(2, 0, 1, 2);
        
        for (int i = 0; i<40000; i++)
        {
            ddIndex.insertIdx(0, i);
        }
        
        
        std::cout << "_s1_ " << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(3));
        std::cout << "_s2_ " << std::endl;
        
        
        for (int i = 0; i<39950; i++)
        {
            ddIndex.deleteIdx(0);
        }
        
        std::cout << "_done_ " << std::endl;
    }
    
    
    static void testDDIndex3(bool hasPersistData)
    {
        if (!hasPersistData) system("rm -r data");
        
        {
            DDIndex<unsigned long, unsigned long> ddIndex(2, 0, 1, 2);
            bool succeeded;
            
            std::list<unsigned long> refList;
            
            for (int i = 0; i<10000; i++)
            {
                if (!hasPersistData) ddIndex.insertIdx(0, i);
                refList.push_front(i);
            }
            
            std::cout << "__sleep " << std::endl;
            std::this_thread::sleep_for(std::chrono::seconds(2));
            std::cout << "______ " << std::endl;
            
            int count = 0;
            int index=0;
            for(auto iter = refList.begin(); iter != refList.end(); iter++)
            {
                unsigned long ddVal = ddIndex.get(index, succeeded);
                
                //std::cout << "val__ " << ddVal << std::endl;
                /*
                if (*iter != ddVal)
                {
                    std::cout << "val__ " << ddVal << "  --  " << *iter << std::endl;
                    break;
                }
                */
                assert(*iter == ddVal);
                
                index++;
                
                if (count % 1000 == 0) std::cout << "__count__" << count << std::endl;
                count++;
            }
            
            std::cout << "_end_____ " << std::endl;
            
        }
        
    }
    
    static void testDDIndex2(bool hasPersistData)
    {
        if (!hasPersistData) system("rm -r data");
        
        DDIndex<unsigned long, unsigned long> ddIndex(1, 0, 1, 2);
        bool succeeded;
        
        std::list<unsigned long> refList;
        
        for (int i = 0; i<10000; i++)
        {
            if (!hasPersistData) ddIndex.insertIdx(0, i);
            refList.push_front(i);
        }
        
        if (!hasPersistData) ddIndex.deleteIdx(0);
        refList.pop_front();
        
        if (!hasPersistData) ddIndex.deleteIdx(6);
        
        auto itr = refList.begin();
        std::advance(itr,6);
        refList.erase(itr);
        
        //std::cout << "_11____" << std::endl;
        
        //if (!hasPersistData) ddIndex.mapFunctsDBug();
        
        //std::cout << "_22____" << std::endl;
        
        
        if (!hasPersistData) ddIndex.deleteIdx(6);
        
        itr = refList.begin();
        std::advance(itr,6);
        refList.erase(itr);
        
        //if (!hasPersistData) ddIndex.mapFunctsDBug();
        
        int index=0;
        for(auto iter = refList.begin(); iter != refList.end(); iter++)
        {
            unsigned long ddVal = ddIndex.get(index, succeeded);
            
            //std::cout << "  ___ "  << ddVal << "  __11___ " << *iter << std::endl;
            
            assert(*iter == ddVal);
            
            index++;
        }
        
        std::cout << "____done____" << std::endl;
    }

    static void testDDIndex(bool hasPersistData)
    {
        if (!hasPersistData) system("rm -r data");
        
        DDIndex<unsigned long, unsigned long> ddIndex(1, 0, 1, 2);
        bool succeeded;
        
        std::list<unsigned long> refList;
        
        
        if (!hasPersistData)  ddIndex.insertIdx(0,101);
        refList.push_back(101);
        
        if (!hasPersistData) ddIndex.insertIdx(1,102);
        refList.push_back(102);
        
        if (!hasPersistData) ddIndex.insertIdx(2,104);
        refList.push_back(104);
        
        
        std::cout << "_s1_ " << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(1));
        std::cout << "_s2_ " << std::endl;
        
        if (!hasPersistData) ddIndex.insertIdx(0,11111);
        refList.push_front(11111);
        
        if (!hasPersistData) ddIndex.insertIdx(0,22222);
        refList.push_front(22222);
        
        if (!hasPersistData) ddIndex.deleteIdx(0);
        refList.pop_front();
        
        if (!hasPersistData) ddIndex.deleteIdx(0);
        refList.pop_front();
        
        int index=0;
        for(auto iter = refList.begin(); iter != refList.end(); iter++)
        {
            unsigned long ddVal = ddIndex.get(index, succeeded);
            assert(*iter == ddVal);
            
            index++;
        }
        
        std::cout << "____done____" << std::endl;
    }
    
    /*
    static void testWithUpdateDDIndex(bool hasPersistData)
    {
        if (!hasPersistData) system("rm -r data");
        
        DDIndex<unsigned long, unsigned long> ddIndex(1, 0, 1, 2);
        bool succeeded;
        
        //std::list<unsigned long> refList;
        
        if (!hasPersistData)  ddIndex.insertIdx(0,10);
        if (!hasPersistData)  ddIndex.insertIdx(0,10);
        
        std::cout << "_sleep_start_" << std::endl;
        std::this_thread::sleep_for(std::chrono::microseconds(10));
        std::cout << "_sleep_end_" << std::endl;
        
        if (!hasPersistData)  ddIndex.insertIdx(0,10);
        
        if (!hasPersistData)  ddIndex.insertIdx(0,101);
        
        if (!hasPersistData)  ddIndex.updateIdx(0, 102);
        
        
        if (!hasPersistData)  ddIndex.updateIdx(1, 9999);
        
        if (!hasPersistData)  ddIndex.insertIdx(3,101111);
        
        //if (!hasPersistData)  ddIndex.deleteIdx(3);
        
        //refList.push_back(102);
        
        
        
        
        for (int i=0; i<ddIndex.size(); i++)
        {
            std::cout << "_tt_" << ddIndex.get(i, succeeded) << std::endl;
            
        }
        
        
        
    }
    */
    
    
    static void testDDMMapAllocator()
    {
        DDMMapAllocator<unsigned long long>* mapAlloc = DDMMapAllocator<unsigned long long>::SHARED();
        
        auto handle = mapAlloc->getHandleFromDataStore<unsigned long long, UserDataHeader>(1, 0);
        //handle->presistVal(0, 12345);
    
        std::cout << "__ddd " << handle->getVal(0) << std::endl;
    }
    
    /*
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
    */
    
    static void testDDLoopReduce()
    {
        //std::cout << "numb of threads " << std::thread::hardware_concurrency << std::endl;
        
        
        DDLoopReduce<long> loopReduce(8);
        
        std::mutex mutex;
        std::mutex* mutexRef = &mutex;
        
        loopReduce.reduce([mutexRef] (long inIdx) -> void
        {
            mutexRef->lock();
            std::cout << "idx " << inIdx << std::endl;
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            mutexRef->unlock();
            
        }, 1000, 10);
    }
    
    static void testDDSpawn()
    {
        DDSpawn ddSpawn(8);
    
        std::mutex mutex;
        //std::mutex* mutexRef = &mutex;
        
        for (int i=0; i<40; i++)
        {
            ddSpawn.runAsyncBlocking([&mutex]()
            {
                //std::this_thread::sleep_for(std::chrono::milliseconds(10));
                
                mutex.lock();
                std::cout << "test " << std::endl;
                mutex.unlock();
            });
            
            ddSpawn.joinAllThreads();
        }
    
        std::cout << "exit " << std::endl;
    }
    
    class TestCacheObj
    {
    public:
        
        TestCacheObj() :
            i(0),
            k(0)
        {}
        
        TestCacheObj(int t) :
            i(t)
        {}
        
        int i;
        int k;
    };
    
    
    static void testDDDeleteField()
    {
        DDDeleteField<unsigned int> m;
        
        
        m.addIdx(7);
        m.addIdx(7);
        
        m.addIdx(5);
        
        m.addIdx(3);
        m.addIdx(3);
        
        for (int i=0; i<8; i++)
        {
            std::cout << "_idx_ " << i << " __ " << m.eval(i) << std::endl;
        }
        
        std::cout << " ___ " << std::endl;
        
        m.fieldItr.startItr();
        
        for (int i=0; i<8; i++)
        {
            auto idx = m.fieldItr.itrEvalAndStep();
            
            std::cout << "_idx_ " << i << " __ " << std::get<0>(idx) << std::endl;
        }
        
    }

    static void testDDInsertField()
    {
        DDInsertField<unsigned int, TestCacheObj> m;
        
        
        m.addIdx(3, TestCacheObj(30));
        
        m.addIdx(7, TestCacheObj(70));
        
        m.addIdx(5, TestCacheObj(50));
        m.addIdx(5, TestCacheObj(51));
        
        
        //m.debugPrint();
        
        
        TestCacheObj testCacheObj;
        bool hasCacheElement;
        
        for (int i=0; i<10; i++)
        {
            auto idx = m.eval(i, hasCacheElement, testCacheObj);
            
            std::cout << "_idx_ " << i << " __ " << idx << " __ " << hasCacheElement << " _elid_ " << testCacheObj.i << std::endl;
        }
        
        std::cout << " ___ " << std::endl;
        
        
        m.fieldItr.startItr();
        
        TestCacheObj testCacheObj2;
        
        /*
        for (int i=0; i<10; i++)
        {
            auto idx = m.fieldItr.itrEvalAndStep(hasCacheElement, testCacheObj2);
        
            std::cout << "_idx_ " << i << " __ " << idx << " __ " << hasCacheElement << " _elid_ " << testCacheObj2.i << std::endl;
        }
        */
    }
    
    static void testDDField()
    {
        DDField<unsigned int, TestCacheObj> field;
        
        
        field.deleteIdx(0);
        
        field.insertIdx(1, TestCacheObj(0));
        field.insertIdx(4, TestCacheObj(1));
        
        field.deleteIdx(4);
        
        field.insertIdx(5, TestCacheObj(2));
        
        
        /*
        field.insertIdx(0, TestCacheObj(2));
        field.insertIdx(0, TestCacheObj(3));
        */
        
        //field.deleteIdx(0);
        //field.deleteIdx(1);
        //field.deleteIdx(3);
        
        //field.deleteIdx(1);
        
        
        /*
         __oo
         _idx_  1 __ 0
         __oo
         __oo
         _idx_  4 __ 1
         _idx_  5 __ 2
         __oo
        */
        
        
        TestCacheObj testCacheObj;
        bool hasCacheElement;
        
        for (int i=0; i<10; i++)
        {
            testCacheObj.i = -1;
            auto idx = field.eval(i, hasCacheElement, testCacheObj);
            
            //std::cout << "__GG " << idx << std::endl;
            //std::cout << "__oo_ " << idx << std::endl;
            
            if (!hasCacheElement)
            {
                std::cout << i << "__oo_ " << idx << std::endl;
            }
            else
            {
                std::cout << i << " _idx_  " << idx << " __ " << testCacheObj.i << std::endl;
            }
            
        }
    }
    
};

#endif
