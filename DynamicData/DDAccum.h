//
//  DDAccum.h
//  DynamicData
//
//  Created by mich2 on 9/11/12.
//  Copyright (c) 2012 -. All rights reserved.
//

#ifndef DynamicData_DDAccum_h
#define DynamicData_DDAccum_h

#include <deque>

template<typename IdxType>
class DDAccum
{
private:
    class IdxYVal
    {
    public:
        DDContinuousID<IdxType> cid;
        IdxType accCount;
        IdxType key;
    };
    
    class ClosureHolder
    {
    public:
        std::function<int (IdxType key)> closure;
        DDContinuousID<IdxType> cid;
    };
    
public:
    
    DDAccum(size_t scopeVal, size_t idVal1, size_t idVal2, size_t idVal3) :
        _ddIndex(scopeVal, idVal1, idVal2, idVal3),
        _currCid(0)
    {
        
    }
    
    DDAccum(const DDAccum&) = delete;
    const DDAccum& operator=(const DDAccum&) = delete;
    
    IdxType accumCount(IdxType key, bool& exists)
    {
        std::unique_lock<std::mutex> lock(_mutex);
        
        IdxType accCount;
        DDContinuousID<IdxType> cid;
        IdxType idx;
        IdxType prevIdx;
        IdxYVal idxYVal;
        
        //OPT binary search here.
        findIdx(key, exists, accCount, cid, idx, prevIdx, idxYVal);
        updateAccCount(key, idxYVal);
        
        //this is wrong!!!
        return idxYVal.accCount;
    }
    
    void insertKey(IdxType key, bool& notUnique)
    {
        std::unique_lock<std::mutex> lock(_mutex);
        
        notUnique = false;
        
        bool exists;
        IdxType accCount;
        DDContinuousID<IdxType> cid;
        IdxType idx;
        IdxType prevIdx;
        IdxYVal idxYVal;
        
        //OPT binary search here.
        findIdx(key, exists, accCount, cid, idx, prevIdx, idxYVal);
        
        if (exists)
        {
            notUnique = true;
        }
        else
        {
            ClosureHolder cHolder;
            
            cHolder.cid = _currCid;
            cHolder.closure = [key] (IdxType inKey) -> int
            {
                int acc = 0;
                
                if (inKey >= key) acc++;
                
                return acc;
            };
            _accumStack.push_back(cHolder);
            
            IdxYVal yVal;
            yVal.cid = _currCid;
            yVal.accCount = 1;
            yVal.key = key;
            
            if (prevIdx == 0) _ddIndex.insertIdx(0, yVal);
            else
            {
                bool succeeded = false;
                IdxYVal prevYVal = _ddIndex.get(prevIdx, succeeded);
                assert(succeeded);

                IdxType accCount = prevYVal.accCount;
                updateAccCount(key, prevYVal);
                
                yVal.cid = _currCid;
                yVal.accCount = accCount;
                
                _ddIndex.insertIdx(prevIdx+1, yVal);
            }
        
            _currCid.incr();
        }
    }
    
    void deleteKey(IdxType key, bool& notFound)
    {
        std::unique_lock<std::mutex> lock(_mutex);
        
        bool exists;
        IdxType accCount;
        DDContinuousID<IdxType> cid;
        IdxType idx;
        IdxType prevIdx;
        
        //OPT binary search here.
        findIdx(key, exists, accCount, cid, idx, prevIdx);
        
        if (!exists) notFound = true;
        else
        {
            _ddIndex.deleteIdx(idx);
        }
        
        ClosureHolder cHolder;
        
        cHolder.cid = _currCid;
        cHolder.closure = [key] (IdxType inKey) -> int
        {
            int acc = 0;
            
            if (inKey >= key) acc--;
            
            return acc;
        };
        _accumStack.push_back(cHolder);
    }
 
private:
    DDIndex<IdxType, IdxYVal> _ddIndex;
    std::deque<ClosureHolder> _accumStack;
    DDContinuousID<IdxType> _currCid;
    std::mutex _mutex;
    
    
    void updateAccCount(IdxType key, IdxYVal idxYVal)
    {
        if (_accumStack.size() > 0)
        {
            IdxType offset = idxYVal.cid.diff(_accumStack[0].cid);
            
            IdxType index;
            for (IdxType i=0; i<_accumStack.size() - offset; i++)
            {
                index = i+offset;
                idxYVal.accCount += _accumStack[index](key);
            }
        }
        
        idxYVal.cid = _currCid;
    }
    
    void findIdx(IdxType key, bool& exists, IdxType& accCount, DDContinuousID<IdxType>& cid, IdxType& idx, IdxType& prevIdx, IdxYVal& idxYVal)
    {
        exists = false;
        
        IdxType size = _ddIndex.size();
        IdxType  currIdx = size / 2;
        IdxType minIdx = 0;
        IdxYVal yVal;
        
        if (size > 0)
        {
            bool done = false;
            while (true)
            {
                bool succeeded;
                yVal = _ddIndex.get(currIdx, succeeded);
                
                if (!done) assert(succeeded);
            
                if (key == yVal.key)
                {
                    exists = true;
                    idx = currIdx;
                    idxYVal = yVal;
                    
                    break;
                }
                else if (size == 1)
                {
                    break;
                }
                else if (done)
                {
                    if (succeeded && key == yVal.key)
                    {
                        exists = true;
                        idx = currIdx;
                        accCount = yVal.accCount;
                        cid = yVal.cid;
                        idxYVal = yVal;
                    }
                    else
                    {
                        prevIdx = currIdx;
                    }
                    
                    break;
                }
                else if (key > yVal.key)
                {
                    IdxType currMaxIdx = currIdx + currIdx - minIdx;
                    minIdx = currIdx;
                    
                    IdxType diff = currMaxIdx - minIdx;
                    
                    if (diff == 1)
                    {
                        currIdx += 1;
                        done = true;
                    }
                    else currIdx = (diff) / 2 + minIdx;
                }
                else if (key < yVal.key)
                {
                    IdxType currMaxIdx = currIdx;
                    IdxType diff = currMaxIdx - minIdx;
                    
                    if (diff == 1)
                    {
                        currIdx -= 1;
                        done = true;
                    }
                    else currIdx = (currMaxIdx - minIdx) / 2 + minIdx;
                }
            }
        }
    }
};

#endif
