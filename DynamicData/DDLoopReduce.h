//
//  DDLoopReduce.h
//  DynamicData
//
//  Created by mich2 on 9/6/12.
//  Copyright (c) 2012 -. All rights reserved.
//

#ifndef DynamicData_DDLoopReduce_h
#define DynamicData_DDLoopReduce_h

#include <list>
#include "DDSpawn.h"

template<typename IdxType>
class DDLoopReduce
{
public:
    
    DDLoopReduce(size_t maxNumOfThreads) :
        _ddSpawn(maxNumOfThreads)
    {
        
    }
    
    DDLoopReduce(const DDLoopReduce&) = delete;
    const DDLoopReduce& operator=(const DDLoopReduce&) = delete;

    void reduce(std::function<void (IdxType inIdx)> func, IdxType range, IdxType slice)
    {
        _reduceMutex.lock();
        
        assert(range >= slice);
        
        bool done;
        IdxType currIdx = 0;
        IdxType currRange = 0;
        
        while(true)
        {
            currIdx += currRange;
            currRange = slicesRange(currIdx, slice, range, done);
            
            _ddSpawn.runAsyncBlocking([func, currIdx, currRange]()
            {
                IdxType idx;
                for (IdxType i=0; i<currRange; i++)
                {
                    idx = i+currIdx;
                    
                    func(idx);
                }
            });
            
            if (done) break;
        }
        
        _ddSpawn.joinAllThreads();
        
        _reduceMutex.unlock();
    }
    
private:
    std::mutex _reduceMutex;
    DDSpawn _ddSpawn;
    
    void compSliceAsync(IdxType startIdx, IdxType sRange, std::function<void (IdxType inIdx)> func)
    {
        IdxType idx;
        for (IdxType i=0; i<sRange; i++)
        {
            idx = i+startIdx;
            
            func(idx);
        }
    }
    
    IdxType slicesRange(IdxType currIdx, IdxType slice, IdxType totalRange, bool& done)
    {
        done = false;
        
        IdxType sRange = slice;
        
        if (currIdx + sRange > totalRange)
        {
            done = true;
            sRange = totalRange - currIdx;
        }
        
        return sRange;
    }
};

#endif