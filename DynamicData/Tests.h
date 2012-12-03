//
//  Tests.h
//  DynamicData
//
//  Created by mich2 on 8/20/12.
//  Copyright (c) 2012 -. All rights reserved.
//

#ifndef DynamicData_Tests_h
#define DynamicData_Tests_h

#include <set>
#include "MMapWrapper.h"
#include "DDIndex.h"
#include "DDLoopReduce.h"
#include "DDSpawn.h"
#include "MMapWrapper.h"
#include "DDActivePassivePtr.h"
#include "DDInsertField.h"
#include "DDDeleteField.h"
#include "DDField.h"
#include "DDBenchmarks.h"
#include "DDRandomGen.h"
#include "DDBaseSet.h"
#include "DDBaseVec.h"

class Tests
{
public:
    Tests(const Tests&) = delete;
    const Tests& operator=(const Tests&) = delete;

    
//
// Benchmark and Assert Tests
//
    class RunnerConfigAssert
    {
    public:
        typedef unsigned int IndexType;
        typedef IndexType IdxType;
        
        static const IdxType IndexSize = 20000;
        static const bool Assert = true;
        
        static const IdxType RandomReads = IndexSize;
        static const IdxType RandomReadWidth = IndexSize;
        
        static const IdxType SequentialReads = IndexSize;
        
        static const IdxType SequentialWrites = IndexSize;
        
        static const IdxType RandomWrites = IndexSize;
        
        static const IdxType RandomDeleteWrites = 9000;
        
        class IndexObj
        {
        public:
            
            static IndexObj rand()
            {
                static DDRandomGen<unsigned int> randGen = DDRandomGen<unsigned int>();
                
                IndexObj indexObj;
                indexObj._id = randGen.randVal();
                
                return indexObj;
            }
            
            unsigned int identifier()
            {
                return _id;
            }
            
            bool operator== (const IndexObj& other) const
            {
                return _id == other._id;
            }
            
        private:
            unsigned int _id;
            int arr[30];
        };
        
    };
    
    
    static void testAssertConfig()
    {
        system("rm -r data");
        
        DDBenchmarkRunner::runBenchmarks<RunnerConfigAssert>();
    }
    
    
    class RunnerConfigBenchMark
    {
    public:
        typedef unsigned int IndexType;
        typedef IndexType IdxType;
        
        static const IdxType IndexSize = 1000000;
        static const bool Assert = false;
        
        static const IdxType RandomReads = IndexSize;
        static const IdxType RandomReadWidth = IndexSize;
        
        static const IdxType SequentialReads = IndexSize;
        
        static const IdxType SequentialWrites = IndexSize;
        
        static const IdxType RandomWrites = IndexSize;
    
        static const IdxType RandomDeleteWrites = 50000;
        
        class IndexObj
        {
        public:
            
            static IndexObj rand()
            {
                static DDRandomGen<unsigned int> randGen = DDRandomGen<unsigned int>();
                
                IndexObj indexObj;
                indexObj._id = randGen.randVal();
                
                return indexObj;
            }
            
            unsigned int identifier()
            {
                return _id;
            }
            
            bool operator== (const IndexObj& other) const
            {
                return _id == other._id;
            }
            
        private:
            unsigned int _id;
            int arr[30];
        };
        
    };
    
    
    static void testBenchmarks()
    {
        system("rm -r data");
        
        DDBenchmarkRunner::runBenchmarks<RunnerConfigBenchMark>();
    }

    
//
//
// Tests DDBaseSet //
//
//
    template<typename IdxType>
    class BaseElement
    {
    public:
        
        BaseElement() : _base(0) {}
        
        void adjust() const
        {
            _base++;
        }
        
        IdxType base() const
        {
            return _base;
        }

    private:
        mutable IdxType _base;
    };
    
    template<typename IdxType>
    class Element
    {
    public:
        
        Element() = default;
        
        Element(IdxType idx, const Element&& element, const BaseElement<IdxType>& baseElement)
        {
            _relIdx = idx - baseElement.base();
        }
        
        void adjust() const
        {
            _relIdx++;
        }

        IdxType idxImp(const BaseElement<IdxType>& baseElement) const
        {
            //std::cout << "__caa " << baseElement.base() << std::endl;
            return _relIdx + baseElement.base();
        }
        
    private:
        mutable IdxType _relIdx;
    };
    
    
    template<typename IdxType>
    class DBugElement
    {
    public:
        
        class Comperator
        {
        public:
            bool operator() (const DBugElement& lhs, const DBugElement& rhs) const { return lhs.idx() < rhs.idx(); }
        };
        
        DBugElement() : _idx(0) {}
        
        DBugElement(IdxType idx) : _idx(idx) {}
        
        void adjust() const { _idx++; }
        
        IdxType idx() const { return _idx; }
        
    private:
        mutable IdxType _idx;
    };
    
    template<typename IdxType>
    class DBugSet
    {
    private:
        typedef typename DBugElement<IdxType>::Comperator Comp;
        
    public:
        typedef std::set<DBugElement<IdxType>, Comp> SetType;
        typedef typename SetType::iterator SetTypeItr;
        
        bool insert(IdxType idx)
        {
            bool check = false;
            
            if (_refSet.count(idx) == 0)
            {
                auto res = _refSet.insert(DBugElement<IdxType>(idx));
                
                assert(res.second);
                
                res.first++;
                adjust(res.first);
            
                check = true;
            }
            
            return check;
        }
        
        SetTypeItr begin() { return _refSet.begin(); }
        SetTypeItr end() { return _refSet.end(); }
        
        size_t size() { return _refSet.size(); }
        
    private:
        SetType _refSet;
    
        void adjust(SetTypeItr itr)
        {
            while (itr != _refSet.end())
            {
                itr->adjust();
                itr++;
            }
        }
    };
    
    /*
    //DDBaseVec
    static void testDDBaseVec()
    {
        typedef unsigned int IdxType;
        DDBaseVec<IdxType, Element<IdxType>, BaseElement<IdxType>, 49> ddBaseVec;
        
        IdxType idx = 1;
        auto itr = ddBaseVec.upperBound(idx);
        ddBaseVec.insert(itr, idx, Element<IdxType>());
    }
    */
    
    static void testDDBaseVec()
    {
        typedef unsigned int IdxType;
        //typedef DDBaseVec<IdxType, Element<IdxType>, BaseElement<IdxType>, 4> BaseVecType;
        typedef DDBaseSet<IdxType, Element<IdxType>, BaseElement<IdxType>, 4> BaseVecType;
        
        DBugSet<IdxType> dBugSet;
        
        std::unique_ptr<BaseVecType>  ddBasePtr(DDUtils::make_unique<BaseVecType>());
        //auto ddBasePtr = std::move(ddBasePtr2);
        
        
        //DDBaseVec<IdxType, Element<IdxType>, BaseElement<IdxType>, 4> ddBaseSet;
        
        
        
        auto printDDBaseSet = [&ddBasePtr]()
        {
            auto currBaseSetPtr = ddBasePtr->baseBegin();
            bool diplayBase = false;
            if (currBaseSetPtr != ddBasePtr->baseEnd())
            {
                diplayBase = true;
            }
            
            
            std::cout << "leaf " << std::endl;
            for (auto itr = ddBasePtr->begin(); itr != ddBasePtr->end(); itr++)
            {
                if (diplayBase || itr->basePtr() != currBaseSetPtr)
                {
                    std::cout << " base " << std::endl;
                    
                    //std::cout << " base " << itr->basePtr()->baseHandleIdx() << " bb " << itr->basePtr()->base() << " _elems_ " << itr->basePtr()->leafElemCount() << std::endl;
                    
                    
                    currBaseSetPtr = itr->basePtr();
                }
                
                //std::cout << " idx " << itr->idx() << " _baseidx_ " << itr->basePtr()->baseContainerIdx() << std::endl;
                std::cout << " idx " << itr->idx() << std::endl;
                
                diplayBase = false;
            }
        
        };
        
        
        auto checkSets = [&]()
        {
            auto itrDBug = dBugSet.begin();
            
            for (auto itr = ddBasePtr->begin(); itr != ddBasePtr->end(); itr++)
            {
                //std::cout << "__idx " << itr->idx() << "__dbg " << itrDBug->idx() << std::endl;

                assert(itr->idx() == itrDBug->idx());
                itrDBug++;
            }
        
        };
        
        IdxType setSize = 7001;
        IdxType stepSize = 7;
        IdxType idx;
        bool check;
        
        for (IdxType i=0; i<setSize; i++)
        {
            idx = (i*stepSize) % setSize;
            check = dBugSet.insert(idx);
        
            if (check)
            {
                auto itr = ddBasePtr->upperBound(idx);
                ddBasePtr->insert(itr, idx, Element<IdxType>());
            }
        }
        
        
        assert(ddBasePtr->size() == dBugSet.size());
       
        
        //printDDBaseSet();
        
        //std::cout << "-----" << std::endl;
        //std::cout << "" << std::endl;
        
        checkSets();
        
        std::cout << "_done_" << std::endl;
    }
  
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    

/*
 * old tests
 */
    
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
    
    
    class TestCacheObj
    {
    public:
        
        TestCacheObj() :
        i(0),
        k(0),
        k7(1111)
        {}
        
        TestCacheObj(int t) :
        i(t)
        {}
        
        int i;
        int k;
        int k3;
        int k4;
        int k5;
        int k6;
        int k7;
        //int arr[20];
    };
    
    static void testFileSizes()
    {
        system("rm -r data");
        
        std::cout << "_1111_ " << std::endl;
        
        DDIndex<unsigned long, TestCacheObj> ddIndex(2, 0, 1, 2);
        
        for (int i = 0; i<2000000; i++)
        {
            ddIndex.insertIdx(0, i);
        }
        
        /*
         std::cout << "_s1_ " << std::endl;
         std::this_thread::sleep_for(std::chrono::seconds(3));
         std::cout << "_s2_ " << std::endl;
         */
        
        
        for (int i = 0; i<1999999; i++)
        {
            ddIndex.deleteIdx(0);
        }
        
        
        //ddIndex.get(, )
        //k7
        
        std::cout << "_done_ " << std::endl;
    }
    
    
    static void testDDIndex3(bool hasPersistData)
    {
        if (!hasPersistData) system("rm -r data");
        
        {
            DDIndex<unsigned long, unsigned long> ddIndex(2, 0, 1, 2);
            
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
                unsigned long ddVal = ddIndex.get(index);
                
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
        
        
        
        std::cout << "_s1_ " << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(1));
        std::cout << "_s2_ " << std::endl;
        
        
        //if (!hasPersistData) ddIndex.mapFunctsDBug();
        
        int index=0;
        for(auto iter = refList.begin(); iter != refList.end(); iter++)
        {
            unsigned long ddVal = ddIndex.get(index);
            
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
            unsigned long ddVal = ddIndex.get(index);
            assert(*iter == ddVal);
            
            index++;
        }
        
        std::cout << "____done____" << std::endl;
    }
    
    static void testDDMMapAllocator()
    {
        DDMMapAllocator<unsigned long long>* mapAlloc = DDMMapAllocator<unsigned long long>::SHARED();
        
        auto handle = mapAlloc->getHandleFromDataStore<unsigned long long, UserDataHeader>(1, 0);
        //handle->presistVal(0, 12345);
        
        std::cout << "__ddd " << handle->getVal(0) << std::endl;
    }
    
    static void testDDLoopReduce()
    {
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
    
    static void testTempTest(bool hasPersistData)
    {
        if (!hasPersistData) system("rm -r data");
        
        DDIndex<unsigned long, unsigned long> ddIndex(1, 0, 1, 2);
        
        std::list<unsigned long> refList;
        
        
        for (int i=0; i<10; i++)
        {
            ddIndex.insertIdx(i,i);
            refList.push_back(i);
        }
        
        int index=0;
        for(auto iter = refList.begin(); iter != refList.end(); iter++)
        {
            std::cout << "_11res_" << ddIndex.get(index) << " __ " << *iter << std::endl;
            
            index++;
        }
        
        auto del = [&ddIndex, &refList](unsigned long idx)
        {
            ddIndex.deleteIdx(idx);
            auto itr = refList.begin();
            advance(itr, idx);
            refList.erase(itr);
        };
        
        del(5);
        
        del(1);
        //del(4);
        
        
        del(6);
        del(0);
        
        del(1);
        
        //ddIndex.debugPrint();
        //std::cout << "__VV " << std::endl;
        del(1);
        
        //ddIndex.debugPrint();
        
        //del(0);
        
        
        
        
        //del(5);
        //del(1);
        //del(4);
        
        //del(0);
        
        
        index=0;
        for(auto iter = refList.begin(); iter != refList.end(); iter++)
        {
            std::cout << "_res_" << ddIndex.get(index) << " __ " << *iter << std::endl;
            if (ddIndex.get(index) != *iter) std::cout << "ff" << std::endl;
            
            index++;
        }
        
        
        std::cout << "____done____" << std::endl;
    }
    
};

#endif
