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

#include "DDIndex.h"

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
    
    template<size_t StartRange, size_t EndRange>
    class RandomGen
    {
        static_assert(StartRange >= 0 && StartRange < EndRange, "RandomGen ranges not consistent.");
        
    public:
        RandomGen() : _distribution(StartRange, EndRange) {}
        
        IdxType randVal() { return _distribution(RandomGen::engine()); }
        
    private:
        std::uniform_int_distribution<IdxType> _distribution;
        
        static std::mt19937& engine()
        {
            static std::mt19937 engine;
            return engine;
        }
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
            
            //std::cout << "-------------" << duration.count() << " ops " << operations << std::endl;
        }
    };
    
    class DDIndexWrapper
    {
    public:
        
        DDIndexWrapper() : ddIndex(createDDIndex()) {}
        
        DDIndexWrapper(const DDIndexWrapper&) = delete;
        const DDIndexWrapper& operator=(const DDIndexWrapper&) = delete;

        void initDDIndex()
        {
            ddIndex(createDDIndex());
        }
        
        DDIndex<IdxType, StoredType> createDDIndex()
        {
            return DDIndex<IdxType, StoredType>(2, 0, 1, 2);
        }
        
        void fillDDIndex()
        {
            assert(ddIndex.size() <= DDIndexSize);
            
            IdxType currSize = ddIndex.size();
            
            for (int i=0; i<DDIndexSize - currSize; i++)
            {
                ddIndex.insertIdx(i, StoredType::rand());
            }
            
            assert(ddIndex.size() == DDIndexSize);
        }
        
        void clearDDIndex()
        {
            ddIndex.unpersist();
            ddIndex = createDDIndex();
        }
        
        DDIndex<IdxType, StoredType> ddIndex;
    };
    
    template<size_t NumOfWrites>
    class RandomWriteBenchmark
    {
    public:
        
        void run(DDIndexWrapper& ddIndexWrapper, Stats& stats)
        {
            ddIndexWrapper.clearDDIndex();
            
            Duration duration;
            
            IdxType randInsertIdx;
            IdxType indexSize;
            
            auto randGen = RandomGen<0, NumOfWrites>();
            
            for (int i=0; i<NumOfWrites; i++)
            {
                //std::cout << "_ee" << std::endl;
                
                indexSize = ddIndexWrapper.ddIndex.size();
                if (indexSize > 0) randInsertIdx = randGen.randVal() % indexSize;
                else randInsertIdx = 0;
            
                ddIndexWrapper.ddIndex.insertIdx(randInsertIdx, StoredType::rand());
            }
            
            stats.benchmarkRes("RandomWriteBenchmark", duration.elapsed(), NumOfWrites);
        }
    };
    
    template<size_t NumOfWrites>
    class SequentialWriteBenchmark
    {
    public:
        
        void run(DDIndexWrapper& ddIndexWrapper, Stats& stats)
        {
            ddIndexWrapper.clearDDIndex();
            
            Duration duration;
            
            for (int i=0; i<NumOfWrites; i++)
            {
                ddIndexWrapper.ddIndex.insertIdx(i, StoredType::rand());
            }
            
            stats.benchmarkRes("SequentialWriteBenchmark", duration.elapsed(), NumOfWrites);
        }
    };
    
    template<size_t NumOfReads>
    class SequentialReadBenchmark
    {
    public:
        
        void run(DDIndexWrapper& ddIndexWrapper, Stats& stats)
        {
            ddIndexWrapper.fillDDIndex();
            
            std::this_thread::sleep_for(std::chrono::seconds(1));
            
            IdxType idx = RandomGen<0, DDIndexSize>().randVal();
            
            Duration duration;
            
            for (IdxType i = 0; i<NumOfReads; i++)
            {
                if (idx == 0) idx = DDIndexSize - 1;
                idx--;
                
                ddIndexWrapper.ddIndex.get(idx);
            }
            
            stats.benchmarkRes("SequentialReadBenchmark", duration.elapsed(), NumOfReads);
        }
    };
    
    template<size_t NumOfReads, size_t ReadWidth>
    class RandomReadBenchmark
    {
        static_assert(ReadWidth <= DDIndexSize, "RandomReadBenchmark error ReadWidth > DDIndexSize");
        
    public:
        
        void run(DDIndexWrapper& ddIndexWrapper, Stats& stats)
        {
            ddIndexWrapper.fillDDIndex();
            
            std::this_thread::sleep_for(std::chrono::seconds(3));
            
            IdxType startIdx = RandomGen<0, DDIndexSize>().randVal();
            
            auto randGen = RandomGen<0, ReadWidth>();
            
            IdxType rndNumb;
            IdxType idx;
            
            Duration duration;
            
            for (IdxType i = 0; i<NumOfReads; i++)
            {
                rndNumb = randGen.randVal();
                
                if (rndNumb > startIdx) idx = DDIndexSize - (rndNumb - startIdx) - 1;
                else idx = startIdx - rndNumb;
                
                ddIndexWrapper.ddIndex.get(idx);
            }
            
            stats.benchmarkRes("RandomReadBenchmark", duration.elapsed(), NumOfReads);
        }
    };

    class Dummy{};
    
    template<class Dummy>
    static void runAll(DDIndexWrapper& ddIndexWrapper, Stats& stats) {}
    
    template<class Dummy, typename Benchmark, typename... Benchmarks>
    static void runAll(DDIndexWrapper& ddIndexWrapper, Stats& stats)
    {
        Benchmark().run(ddIndexWrapper, stats);
        runAll<Dummy, Benchmarks...>(ddIndexWrapper, stats);
    }
    
public:
    DDBenchmarks() {}
    
    DDBenchmarks(const DDBenchmarks&) = delete;
    const DDBenchmarks& operator=(const DDBenchmarks&) = delete;

    static void runBenchmarks()
    {
        DDIndexWrapper ddIndexWrapper;
        Stats stats;
        
        static const size_t RandomReads = 100000;
        static const size_t SequentialReads = 100000;
        static const size_t SequentialWrites = 100000;
        static const size_t RandomWrites = 100000;
        
        DDBenchmarks::runAll<Dummy,
        
            RandomReadBenchmark<RandomReads, DDIndexSize>,
            SequentialReadBenchmark<SequentialReads>,
            SequentialWriteBenchmark<SequentialWrites>,
            RandomWriteBenchmark<RandomWrites>
        
            //... more benchmarks.
        >(ddIndexWrapper, stats);
    }
};

#endif
