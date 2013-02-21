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
            typename std::list<StoredType>::iterator itr = _list.begin();
            advance(itr, idx);
            _list.erase(itr);
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
    
public:
    
    DDBenchmarks() {}
    
    DDBenchmarks(const DDBenchmarks&) = delete;
    const DDBenchmarks& operator=(const DDBenchmarks&) = delete;

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
    
    template<size_t NumOfWriteDeletes, class IndexHandle>
    class RandomWriteDeleteBenchmark
    {
    public:
        
        void run(IndexHandle& indexHandle, Stats& stats)
        {
            indexHandle.fillDDIndex();
            
            std::this_thread::sleep_for(std::chrono::seconds(1));
            
            Duration duration;
            
            IdxType randDelIdx;
            IdxType randInsertIdx;
            IdxType indexSize;
            
            auto randGen = DDRandomGen<IdxType>(0, indexHandle.size());
            
            for (int i=0; i<NumOfWriteDeletes*2; i++)
            {
                indexSize = indexHandle.size();
                randDelIdx = randGen.randVal() % indexSize;
                
                indexHandle.deleteIdx(randDelIdx);
            }
            
            for (int i=0; i<NumOfWriteDeletes; i++)
            {
                indexSize = indexHandle.size();
                randInsertIdx = randGen.randVal() % indexSize;
                
                indexHandle.insertIdx(randDelIdx, StoredType::rand());
            }
            
            stats.benchmarkRes("RandomWriteDeleteBenchmark", duration.elapsed(), 2*NumOfWriteDeletes);
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
                //TODO check this!
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
};

/*
 RunnerConfig
 
 RunnerConfig::IdxType
 IdxType RunnerConfig::IndexSize

 bool RunnerConfig::Assert
 
 size_t RunnerConfig::RandomReads
 size_t RunnerConfig::RandomReadWidth
 
 size_t RunnerConfig::SequentialReads
 size_t RunnerConfig::SequentialWrites
 size_t RunnerConfig::RandomWrites
 size_t RunnerConfig::RandomDeleteWrites
 
 IndexObj RunnerConfig::IndexObj
*/

/*
 IndexObj
 static IndexObj rand();
*/

class DDBenchmarkRunner
{
public:
    class Dummy {};
    
    template<typename RunnerConfig>
    static void runBenchmarks()
    {
        typedef DDBenchmarks<typename RunnerConfig::IdxType,typename RunnerConfig::IndexObj, RunnerConfig::IndexSize> BenchmarkType;
        
        //BenchmarkType benchmarks;
        
        typedef typename BenchmarkType::template IndexHandleTrait<Dummy, RunnerConfig::Assert>::type IndexHandleType;
    
        
        IndexHandleType ddIndexHandle;
        typename BenchmarkType::Stats stats;
        
        //
        //Benchmark types.
        typedef typename BenchmarkType::template SequentialReadBenchmark<RunnerConfig::SequentialReads, IndexHandleType> SequentialReadBMType;
        typedef typename BenchmarkType::template RandomReadBenchmark<RunnerConfig::RandomReads, RunnerConfig::RandomReadWidth, IndexHandleType> RandomReadBMType;
        typedef typename BenchmarkType::template SequentialWriteBenchmark<RunnerConfig::SequentialWrites, IndexHandleType> SequentialWriteBMType;
        typedef typename BenchmarkType::template RandomWriteBenchmark<RunnerConfig::RandomWrites, IndexHandleType> RandomWriteBMType;
        typedef typename BenchmarkType::template RandomWriteDeleteBenchmark<RunnerConfig::RandomDeleteWrites, IndexHandleType> RandomWriteDeleteBMType;
        //
        //
        
        //Type for checking the index if requested.
        typedef typename BenchmarkType::template CheckHandle<IndexHandleType, RunnerConfig::Assert> CheckHandleType;
        
        for (int i=0; i<10; i++)
        {
            BenchmarkType::template run
            <
            IndexHandleType,
            
            SequentialReadBMType,
            RandomReadBMType,
            SequentialWriteBMType,
            RandomWriteBMType,
            RandomWriteDeleteBMType
            
            //... more benchmarks.
            >(i, ddIndexHandle, stats);
            
            CheckHandleType::check(ddIndexHandle);
        }
    }
};

#endif
