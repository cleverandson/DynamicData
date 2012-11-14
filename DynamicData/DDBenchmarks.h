//
//  DDBenchmarks.h
//  DynamicData
//
//  Created by mich2 on 11/7/12.
//  Copyright (c) 2012 -. All rights reserved.
//

#ifndef DynamicData_DDBenchmarks_h
#define DynamicData_DDBenchmarks_h

#include <random>
#include <functional>
#include <chrono>
#include <iomanip>
#include <list>

#include "DDIndex.h"
#include "DDRandomGen.h"

template<typename IdxType, typename StoredType, size_t DDIndexSize>
class DDBenchmarks
{
private:
    
    typedef std::chrono::microseconds microsec;
    
    class Duration
    {
        typedef std::chrono::high_resolution_clock clock;
        
    public:
        Duration() : _start(clock::now()) {}
        
        microsec elapsed() { return std::chrono::duration_cast<microsec>(clock::now() - _start); }
        
    private:
        clock::time_point _start;
    };
    
    class Stats
    {
    public:
        
        void benchmarkRes(std::string benchmarkName, microsec duration, size_t operations)
        {
            std::cout << std::endl;
            std::cout << "-------------" << std::endl;
            std::cout << "Benchmark " << benchmarkName << std::endl;
            std::cout << "OPS/SEC: " << (long long)(1000000.0 / (float)duration.count() * (float)operations)  << std::endl;
            std::cout << "-------------" << std::endl;
        }
    };
    
    class DDIndexWrapper
    {
    public:
        
        DDIndexWrapper(DDIndex<IdxType, StoredType>&& ddIndex) :
        _ddIndex(std::move(ddIndex))
        {}
        
        StoredType get(IdxType idx)
        {
            return _ddIndex.get(idx);
        }
        
        void insertIdx(IdxType idx, StoredType yValue)
        {
            _ddIndex.insertIdx(idx, yValue);
        }
        
        void deleteIdx(IdxType idx)
        {
            _ddIndex.deleteIdx(idx);
        }
        
        IdxType size() { return _ddIndex.size(); }
        
        void unpersist()
        {
            _ddIndex.unpersist();
        }
        
    protected:
        DDIndex<IdxType, StoredType> _ddIndex;
    };
    
    class DDIndexWrapperAssert
    {
    public:
        
        DDIndexWrapperAssert(DDIndex<IdxType, StoredType>&& ddIndex) :
            _ddIndex(std::move(ddIndex))
        {}
        
        StoredType get(IdxType idx)
        {
            return _ddIndex.get(idx);
        }
        
        void insertIdx(IdxType idx, StoredType yValue)
        {
            typename std::list<StoredType>::iterator itr = _list.begin();
            advance(itr, idx);
            _list.insert(itr, yValue);
            _ddIndex.insertIdx(idx, yValue);
        }
        
        void deleteIdx(IdxType idx)
        {
            _list.erase(idx);
            _ddIndex.deleteIdx(idx);
        }
        
        IdxType size() { return _ddIndex.size(); }
        
        void unpersist()
        {
            _ddIndex.unpersist();
            _list.clear();
        }
        
        void check()
        {
            assert(_list.size() == _ddIndex.size());
        
            unsigned int temp = 0;
            IdxType idx = 0;
            for(auto iter = _list.begin(); iter != _list.end(); iter++)
            {
                assert(*iter == _ddIndex.get(idx));
                
                temp++;
                idx++;
            }
        }
        
    protected:
        std::list<StoredType> _list;
        DDIndex<IdxType, StoredType> _ddIndex;
    };
    
    template<class IndexWrapper>
    class DDIndexHandle : public IndexWrapper
    {
    public:
        
        DDIndexHandle() : IndexWrapper(createDDIndex()) {}
        
        DDIndexHandle(const DDIndexHandle&) = delete;
        const DDIndexHandle& operator=(const DDIndexHandle&) = delete;

        void initDDIndex()
        {
            _ddIndex(createDDIndex());
            
            ddIndex(createDDIndex());
        }
        
        DDIndex<IdxType, StoredType> createDDIndex()
        {
            return DDIndex<IdxType, StoredType>(2, 0, 1, 2);
        }
        
        void fillDDIndex()
        {
            
            
            assert(IndexWrapper::size() <= DDIndexSize);
            
            IdxType currSize = IndexWrapper::size();
            
            for (int i=0; i<DDIndexSize - currSize; i++)
            {
                IndexWrapper::insertIdx(i, StoredType::rand());
            }
            
            assert(IndexWrapper::size() == DDIndexSize);
        }
        
        void clearDDIndex()
        {
            IndexWrapper::unpersist();
            IndexWrapper::_ddIndex = createDDIndex();
        }
    };
    
    
    template<size_t NumOfWrites, class IndexHandle>
    class RandomWriteBenchmark
    {
    public:
        
        void run(IndexHandle& indexHandle, Stats& stats)
        {
            indexHandle.clearDDIndex();
            
            Duration duration;
            
            IdxType randInsertIdx;
            IdxType indexSize;
            
            auto randGen = DDRandomGen<IdxType>(0, NumOfWrites);
            
            for (int i=0; i<NumOfWrites; i++)
            {
                indexSize = indexHandle.size();
                if (indexSize > 0) randInsertIdx = randGen.randVal() % indexSize;
                else randInsertIdx = 0;

                indexHandle.insertIdx(randInsertIdx, StoredType::rand());
            }
            
            stats.benchmarkRes("RandomWriteBenchmark", duration.elapsed(), NumOfWrites);
        }
    };
    
    template<size_t NumOfWrites, class IndexHandle>
    class SequentialWriteBenchmark
    {
    public:
        
        void run(IndexHandle& indexHandle, Stats& stats)
        {
            indexHandle.clearDDIndex();
            
            Duration duration;
            
            for (int i=0; i<NumOfWrites; i++)
            {
                indexHandle.insertIdx(i, StoredType::rand());
            }
            
            stats.benchmarkRes("SequentialWriteBenchmark", duration.elapsed(), NumOfWrites);
        }
    };
    
    template<size_t NumOfReads, class IndexHandle>
    class SequentialReadBenchmark
    {
    public:
        
        void run(IndexHandle& indexHandle, Stats& stats)
        {
            indexHandle.fillDDIndex();
            
            std::this_thread::sleep_for(std::chrono::seconds(1));
            
            IdxType idx = DDRandomGen<IdxType>(0, DDIndexSize).randVal();
            
            Duration duration;
            
            for (IdxType i = 0; i<NumOfReads; i++)
            {
                if (idx == 0) idx = DDIndexSize - 1;
                idx--;
                
                indexHandle.get(idx);
            }
            
            stats.benchmarkRes("SequentialReadBenchmark", duration.elapsed(), NumOfReads);
        }
    };
    
    template<size_t NumOfReads, size_t ReadWidth, class IndexHandle>
    class RandomReadBenchmark
    {
        static_assert(ReadWidth <= DDIndexSize, "RandomReadBenchmark error ReadWidth > DDIndexSize");
        
    public:
        
        void run(IndexHandle& indexHandle, Stats& stats)
        {
            indexHandle.fillDDIndex();
            
            std::this_thread::sleep_for(std::chrono::seconds(3));
            
            IdxType startIdx = DDRandomGen<IdxType>(0, DDIndexSize).randVal();
            
            auto randGen = DDRandomGen<IdxType>(0, ReadWidth);
            
            IdxType rndNumb;
            IdxType idx;
            
            Duration duration;
            
            for (IdxType i = 0; i<NumOfReads; i++)
            {
                rndNumb = randGen.randVal();
                
                if (rndNumb > startIdx) idx = DDIndexSize - (rndNumb - startIdx) - 1;
                else idx = startIdx - rndNumb;
                
                indexHandle.get(idx);
            }
            
            stats.benchmarkRes("RandomReadBenchmark", duration.elapsed(), NumOfReads);
        }
    };
    
    template<size_t Idx, class IndexHandle>
    static void run(size_t index, IndexHandle& indexHandle, Stats& stats) {}
    
    template<size_t Idx, class IndexHandle, typename Benchmark, typename... Benchmarks>
    static void run(size_t index, IndexHandle& indexHandle, Stats& stats)
    {
        if (Idx == index) Benchmark().run(indexHandle, stats);
        run<Idx+1, IndexHandle, Benchmarks...>(index, indexHandle, stats);
    }
    
    template<class IndexHandle, typename... Benchmarks>
    static void run(size_t index, IndexHandle& indexHandle, Stats& stats)
    {
        index = index % sizeof...(Benchmarks);
        
        run<0, IndexHandle, Benchmarks...>(index, indexHandle, stats);
    }
    
public:
    DDBenchmarks() {}
    
    DDBenchmarks(const DDBenchmarks&) = delete;
    const DDBenchmarks& operator=(const DDBenchmarks&) = delete;

    //typedef DDIndexHandle<DDIndexWrapperAssert> IndexHandleAssertType;
    
    class Dummy {};
    
    template<class Dummy, bool Assert>
    class IndexHandleTrait
    {
    public:
        typedef DDIndexHandle<DDIndexWrapper> type;
    };
    
    template<class Dummy>
    class IndexHandleTrait<Dummy, true>
    {
    public:
        typedef DDIndexHandle<DDIndexWrapperAssert> type;
    };
    
    
    template<class IndexHandle, bool Assert>
    class CheckHandle
    {
    public:
        static void check(IndexHandle& handle) {};
    };
    
    template<class IndexHandle>
    class CheckHandle<IndexHandle, true>
    {
    public:
        static void check(IndexHandle& handle)
        {
            handle.check();
        }
    };
    
    static void runBenchmarks()
    {
        typedef DDIndexHandle<DDIndexWrapper> IndexHandleType;
        
        static const bool AssertIndexHandle = false;
        
        typename IndexHandleTrait<Dummy, AssertIndexHandle>::type ddIndexHandle;
        Stats stats;
        
        static const size_t RandomReads = 100000;
        static const size_t SequentialReads = 100000;
        static const size_t SequentialWrites = 100000;
        static const size_t RandomWrites = 100000;
        
        /*
        //assert config.
        static const bool AssertIndexHandle = true;
        
        typename IndexHandleTrait<Dummy, AssertIndexHandle>::type ddIndexHandle;
        Stats stats;
        
        static const size_t RandomReads = 10000;
        static const size_t SequentialReads = 10000;
        static const size_t SequentialWrites = 10000;
        static const size_t RandomWrites = 10000;
        */
        
        
        
        
        /*
        DDBenchmarks::run
        <
        typename IndexHandleTrait<Dummy, AssertIndexHandle>::type,
        
        RandomReadBenchmark<RandomReads, DDIndexSize, typename IndexHandleTrait<Dummy, AssertIndexHandle>::type>,
        SequentialReadBenchmark<SequentialReads, typename IndexHandleTrait<Dummy, AssertIndexHandle>::type>,
        SequentialWriteBenchmark<SequentialWrites, typename IndexHandleTrait<Dummy, AssertIndexHandle>::type>,
        RandomWriteBenchmark<RandomWrites, typename IndexHandleTrait<Dummy, AssertIndexHandle>::type>
        
        //... more benchmarks.
        >(1, ddIndexHandle, stats);
        
        CheckHandle<typename IndexHandleTrait<Dummy, AssertIndexHandle>::type, AssertIndexHandle>::check(ddIndexHandle);
        */
        
        
        
        for (int i=0; i<10; i++)
        {
            DDBenchmarks::run
            <
                typename IndexHandleTrait<Dummy, AssertIndexHandle>::type,
            
            /*
                RandomReadBenchmark<RandomReads, DDIndexSize, typename IndexHandleTrait<Dummy, AssertIndexHandle>::type>,
                SequentialReadBenchmark<SequentialReads, typename IndexHandleTrait<Dummy, AssertIndexHandle>::type>,
                SequentialWriteBenchmark<SequentialWrites, typename IndexHandleTrait<Dummy, AssertIndexHandle>::type>,
            */    
            
                RandomWriteBenchmark<RandomWrites, typename IndexHandleTrait<Dummy, AssertIndexHandle>::type>

                //... more benchmarks.
            >(i, ddIndexHandle, stats);
        
            CheckHandle<typename IndexHandleTrait<Dummy, AssertIndexHandle>::type, AssertIndexHandle>::check(ddIndexHandle);
        }
        
        
        //typedef DDIndexHandle<DDIndexWrapperAssert> IndexHandleType;
    }
};

#endif
