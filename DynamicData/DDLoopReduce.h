/*
 
    Copyright (c) 2013, Clever & Son
    All rights reserved.

    Redistribution and use in source and binary forms, with or without modification, are
    permitted provided that the following conditions are met:

    Redistributions of source code must retain the above copyright notice, this list of
    conditions and the following disclaimer.
    Redistributions in binary form must reproduce the above copyright notice, this list of
    conditions and the following disclaimer in the documentation and/or other materials
    provided with the distribution.
    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND
    CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
    INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
    MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
    CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
    SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
    NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
    CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
    STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
    ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
    ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 
*/


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