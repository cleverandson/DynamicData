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