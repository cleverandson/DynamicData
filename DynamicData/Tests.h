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
};

#endif
