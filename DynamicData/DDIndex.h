//
//  DDIndex.h
//  DynamicData
//
//  Created by mich2 on 8/28/12.
//  Copyright (c) 2012 -. All rights reserved.
//

#ifndef DynamicData_DDIndex_h
#define DynamicData_DDIndex_h

#include <list>
#include <mutex>

#include "DDMMapAllocator.h"
#include "DDContinuousID.h"

template<typename IdxType, typename YType>
class DDIndex
{
private:

    class DoubleSyncedMMapWrapper
    {
    private:
        
        class MMapHeader
        {
        public:
            MMapHeader() :
                done(false)
            {
                
            }
            
            bool done;
        };
        
    public:
        DoubleSyncedMMapWrapper(size_t scopeVal, size_t idVal1, size_t idVal2) :
            _activeMapIdx(0)
        {
            _mmapWrapper1 = DDMMapAllocator<IdxType>::SHARED()->template getHandleFromDataStore<IdxType, MMapHeader>(scopeVal, idVal1);
            _mmapWrapper2 = DDMMapAllocator<IdxType>::SHARED()->template getHandleFromDataStore<IdxType, MMapHeader>(scopeVal, idVal2);
            
            bool check = false;
            MMapHeader header = _mmapWrapper1->getUserDataHeader();
            
            if (header.done)
            {
                check = true;
                _activeMapIdx = 1;
            }
            
            if (!check)
            {
                header = _mmapWrapper2->getUserDataHeader();
                
                if (header.done)
                {
                    _activeMapIdx = 2;
                }
            }
        }
        
        IdxType get(IdxType idx)
        {
            IdxType retIdx;
            
            if (_activeMapIdx == 0)
            {
                retIdx = idx;
            }
            else if (_activeMapIdx == 1)
            {
                retIdx = _mmapWrapper1->getVal(idx);
            }
            else
            {
                retIdx = _mmapWrapper2->getVal(idx);
            }
            
            return retIdx;
        }
        
        IdxType size()
        {
            IdxType size;
            
            if (_activeMapIdx == 0)
            {
                size = 0;
            }
            else if (_activeMapIdx == 1)
            {
                size = _mmapWrapper1->size();
            }
            else
            {
                size = _mmapWrapper2->size();
            }
            
            return size;
        }
        
        void persist(IdxType idx, IdxType mappedIdx)
        {
            if (_activeMapIdx == 0 || _activeMapIdx == 1)
            {
                _mmapWrapper2->persist(idx, mappedIdx);
            }
            else
            {
                _mmapWrapper1->persist(idx, mappedIdx);
            }
        }
        
        //TODO check this potentially dangerous when application crashes.
        void switchMMaps(IdxType nextSize)
        {
            if (_activeMapIdx == 0 || _activeMapIdx == 1)
            {
                MMapHeader header = _mmapWrapper1->getUserDataHeader();
                header.done = false;
                _mmapWrapper1->saveUserDataHeader(header);
                
                header = _mmapWrapper2->getUserDataHeader();
                header.done = true;
                header.size = nextSize;
                _mmapWrapper2->saveUserDataHeader(header);
                
                _activeMapIdx = 2;
            }
            else
            {
                MMapHeader header = _mmapWrapper2->getUserDataHeader();
                header.done = false;
                _mmapWrapper2->saveUserDataHeader(header);
                
                header = _mmapWrapper1->getUserDataHeader();
                header.done = true;
                header.size = nextSize;
                _mmapWrapper1->saveUserDataHeader(header);
                
                _activeMapIdx = 1;
            }
        }
        
    private:
        std::unique_ptr<MMapWrapper<IdxType, IdxType, MMapHeader>> _mmapWrapper1;
        std::unique_ptr<MMapWrapper<IdxType, IdxType, MMapHeader>> _mmapWrapper2;
        
        unsigned int _activeMapIdx;
    };
    
    class FuncData
    {
    public:
        std::function<IdxType (IdxType inIdx)> closure;
        std::function<IdxType (IdxType inIdx)> invClosure;
        DDContinuousID<unsigned long> cid;
    };
    
    class YValMapHeader { };
    
public:
    
    DDIndex(size_t scopeVal, size_t idVal1, size_t idVal2, size_t idVal3) :
        _doubleSyncedMMapWrapper(scopeVal, idVal1, idVal2),
        _yValMMapWrapper(DDMMapAllocator<IdxType>::SHARED()->template getHandleFromDataStore<YType, YValMapHeader>(scopeVal, idVal3)),
        _size(_doubleSyncedMMapWrapper.size()),
        _lastCid(0)
    {
        
    }
    
    DDIndex(const DDIndex&) = delete;
    const DDIndex& operator=(const DDIndex&) = delete;
    
    YType get(IdxType idx, DDContinuousID<unsigned long> cid, bool& succeeded)
    {
        YType yVal;
        
        _mutex.lock();
        
        succeeded = cid.value() == _lastCid.value();
        
        if (idx < _size)
        {
            yVal = _yValMMapWrapper->getVal(eval(idx));
        }
        
        _mutex.unlock();
        
        return yVal;
    }
    
    //TODO overrun check!
    void insertIdx(IdxType idx, YType value)
    {
        _mutex.lock();
        
        IdxType yIdx = _yValMMapWrapper->size();
        _yValMMapWrapper->persistVal(yIdx, value);
        
        
        FuncData funcData;
        
        int size = _size + 1;
        funcData.closure = [idx,size] (IdxType inIdx) -> IdxType
        {
            IdxType retIdx = inIdx;
            
            if (inIdx == idx)
            {
                retIdx = yIdx;
            }
            else if (inIdx > idx && inIdx < size)
            {
                retIdx = inIdx -1;
            }
            
            return retIdx;
        };
        
    
        //TODO optimize size - 1 and idx - 1
        funcData.invClosure = [idx, size] (IdxType inIdx) -> IdxType
        {
            IdxType retIdx = inIdx;
            
            if (inIdx == yIdx)
            {
                retIdx = idx;
            }
            else if (inIdx > idx - 1 && inIdx < size -1)
            {
                retIdx = inIdx + 1;
            }
            
            return retIdx;
        };
    
        
        funcData.cid = _lastCid;
        
        _lastCid++;
        _size = size;
        
        _funcDataVec.push_back(funcData);
        
        _mutex.unlock();
    }
    
    //TODO overrun check!
    void deleteIdx(IdxType idx)
    {
        _mutex.lock();
        
        FuncData funcData;
        
        int size = _size - 1;
        
        funcData.closure = [idx, size] (IdxType inIdx) -> IdxType
        {
            IdxType retIdx = inIdx;
            
            if (inIdx == size-1)
            {
                retIdx = idx;
            }
            else if (inIdx >= idx && inIdx < size-1)
            {
                retIdx = inIdx + 1;
            }
        };
        
        
        funcData.invClosure = [idx, size] (IdxType inIdx) -> IdxType
        {
            IdxType retIdx = inIdx;
            
            if (inIdx == idx)
            {
                retIdx = size-1;
            }
            else if (inIdx > idx && inIdx < size)
            {
                retIdx = inIdx - 1;
            }
            
            return retIdx;
        };
        
        
        funcData.cid = _lastCid;
        
        _lastCid++;
        
        _funcDataVec.push_back(funcData);
        
        _size = size;
        
        _mutex.unlock();
    }
    
    IdxType size()
    {
        IdxType size;
        
        _mutex.lock();
        size = _size;
        _mutex.unlock();
        
        return size;
    }
    
private:
    //XVal Wrapper.
    DoubleSyncedMMapWrapper _doubleSyncedMMapWrapper;
    
    //YVal Wrapper.
    std::unique_ptr<MMapWrapper<IdxType, YType, YValMapHeader>> _yValMMapWrapper;
    
    IdxType _size;
    
    
    std::list<FuncData> _funcDataVec;
    DDContinuousID<unsigned long> _lastCid;
    std::mutex _mutex;
    
    //optimization.
    //std::array<int, 3>
    
    void asyncCaching()
    {
        std::list<FuncData> cpyFuncDataVec;
        IdxType size;
        
        _mutex.lock();
        
        //OPTIMIZATION?
        cpyFuncDataVec = _funcDataVec;
        _size = size;
        
        _mutex.unlock();
        
        IdxType idx;
        
        for (int i=0; i<size; i++)
        {
            idx = eval(i);
            _doubleSyncedMMapWrapper->persist(i, idx);
        }
        
        
        //shrink _doubleSyncedMMapWrapper if needed.
        
        
        _mutex.lock();
        
        
        
        /*
        //remove cached functs.
        FuncData funcData;
        while (true)
        {
            funcData = _funcDataVec.front;
            _funcDataVec.pop_front();
        
            
        }
        
        //garbage collect! necessary?
        //switchMMaps(IdxType nextSize)
        */
        
        _mutex.unlock();
    }
    
    IdxType eval(IdxType idx)
    {
        IdxType retVal;
        
        for(auto& funcData : _funcDataVec)
        {
            idx = funcData.closure(idx);
        }
        
        return _doubleSyncedMMapWrapper.get(idx);
    }
    
    IdxType invEval(IdxType idx, DDContinuousID<unsigned long> cacheCid)
    {
        if (_doubleSyncedMMapWrapper.size() > idx)
        {
            idx = _doubleSyncedMMapWrapper.get(idx);
        }
        
        for (auto rit=_funcDataVec.rbegin() ; rit < _funcDataVec.rend(); ++rit)
        {
            idx = rit->invClosure(idx);
            
            if (rit->cid == cacheCid) break;
        }
        
        return idx;
    }
    
};

#endif
