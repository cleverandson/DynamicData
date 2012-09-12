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
#include <atomic>
#include <thread>

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
        
        IdxType getBack(IdxType idx)
        {
            IdxType retIdx;
            
            if (_activeMapIdx == 0)
            {
                retIdx = idx;
            }
            else if (_activeMapIdx == 1)
            {
                retIdx = _mmapWrapper2->getVal(idx);
            }
            else
            {
                retIdx = _mmapWrapper1->getVal(idx);
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
                _mmapWrapper2->persistVal(idx, mappedIdx);
            }
            else
            {
                _mmapWrapper1->persistVal(idx, mappedIdx);
            }
        }
        
        void shrinkSize(IdxType size)
        {
            if (_activeMapIdx == 0 || _activeMapIdx == 1)
            {
                _mmapWrapper2->shrinkSize(size);
            }
            else
            {
                _mmapWrapper1->shrinkSize(size);
            }
        }
        
        
        //TODO check this potentially dangerous when application crashes.
        void switchMMaps()
        {
            if (_activeMapIdx == 0 || _activeMapIdx == 1)
            {
                MMapHeader header = _mmapWrapper1->getUserDataHeader();
                header.done = false;
                _mmapWrapper1->saveUserDataHeader(header);
                
                header = _mmapWrapper2->getUserDataHeader();
                header.done = true;
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
        std::function<IdxType (IdxType inIdx, bool& hasInsValue, YType& insVal, bool& hasDelValue, IdxType& delVal, bool& hasUpdVal, YType& updVal)> closure;
    };
    
    class YValMapHeader { };
    
public:
    
    DDIndex(size_t scopeVal, size_t idVal1, size_t idVal2, size_t idVal3) :
        _doubleSyncedMMapWrapper(scopeVal, idVal1, idVal2),
        _yValMMapWrapper(DDMMapAllocator<IdxType>::SHARED()->template getHandleFromDataStore<YType, YValMapHeader>(scopeVal, idVal3)),
        _size(_doubleSyncedMMapWrapper.size()),
        _shoutdownCount(0),
        _reduceAndSwapThread(&DDIndex::reduceAndSwapMap,this)
    {
    
    }
    
    ~DDIndex()
    {
        _mutex.lock();
        _mutex.unlock();
        
        _shoutdownCount = 2;
        
        _reduceAndSwapThread.join();
    }
    
    DDIndex(const DDIndex&) = delete;
    const DDIndex& operator=(const DDIndex&) = delete;
    
    YType get(IdxType idx, bool& succeeded)
    {
        succeeded = false;
        YType yVal;
        
        //TODO implement like std::unique_lock<std::mutex> lock(_mutex);
        _mutex.lock();
        
        bool hasInsValue=false;
        YType insYValue;
        bool hasUpdValue=false;
        YType updYValue;
        
        if (idx < _size)
        {
            idx = eval(idx, _funcDataVec, hasInsValue, insYValue, false, hasUpdValue, updYValue);
            
            if (hasInsValue)
            {
                yVal = insYValue;
            }
            else if (hasUpdValue)
            {
                yVal = updYValue;
            }
            else
            {
                _yValMutex.lock();
                yVal = _yValMMapWrapper->getVal(idx);
                _yValMutex.unlock();
            }
            
            succeeded = true;
        }
        
        _mutex.unlock();
        
        return yVal;
    }
    
    void updateIdx(IdxType idx, YType yValue, bool& succeeded)
    {
        /*
        succeeded = false;
        
        _mutex.lock();
        
        bool hasInsValue=false;
        YType insYValue;
        bool shouldUpdate = true;
        
        if (idx < _size)
        {
            idx = eval(idx, _funcDataVec, hasInsValue, insYValue, shouldUpdate, yValue);
            
            if (hasInsValue)
            {
                //yVal = insYValue;
            }
            else
            {
                //probably wrong sync!!!
                _yValMutex.lock();
                _yValMMapWrapper->persistVal(idx, yValue);
                _yValMutex.unlock();
            }
            
            succeeded = true;
        }
        
        _mutex.unlock();
        */
    }
    
    void insertIdx(IdxType idx, YType yValue)
    {
        if (!_shoutdownCount)
        {
            assert(idx < _size + 1);
            
            _mutex.lock();
            
            FuncData funcData;
            
            _size++;
            funcData.closure = [idx, yValue] (IdxType inIdx, bool& hasInsValue, YType& insVal, bool& hasDelValue, IdxType& delVal, bool& hasUpdVal, YType& updVal) -> IdxType
            {
                hasInsValue = false;
                hasDelValue = false;
                hasUpdVal = false;
                
                IdxType retIdx = inIdx;
                
                if (inIdx == idx)
                {
                    insVal = yValue;
                    hasInsValue = true;
                }
                else if (inIdx > idx)
                {
                    retIdx = inIdx -1;
                }
                
                return retIdx;
            };
            
            _funcDataVec.push_front(funcData);
            
            _mutex.unlock();
        }
    }
    
    void deleteIdx(IdxType idx)
    {
        if (!_shoutdownCount)
        {
            assert(idx < _size);
            
            _mutex.lock();
            
            FuncData funcData;
           
            _size--;
            
            funcData.closure = [idx] (IdxType inIdx, bool& hasInsValue, YType& insVal, bool& hasDelValue, IdxType& delVal, bool& hasUpdVal, YType& updVal) -> IdxType
            {
                hasInsValue = false;
                hasUpdVal = false;
                hasDelValue = true;
                delVal = idx;
                
                
                IdxType retIdx = inIdx;
                
                if (inIdx >= idx)
                {
                    retIdx = inIdx + 1;
                }
                
                return retIdx;
            };
            
            _funcDataVec.push_front(funcData);
            
            _mutex.unlock();
        }
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
    std::mutex _yValMutex;
    
    IdxType _size;
    std::list<FuncData> _funcDataVec;
    std::mutex _mutex;
    std::atomic<int> _shoutdownCount;
    std::thread _reduceAndSwapThread;
    
    
    void reduceAndSwapMap()
    {
        while(true)
        {
            size_t numberOfFuncts;
            _mutex.lock();
            numberOfFuncts = _funcDataVec.size();
            _mutex.unlock();
            
            if (numberOfFuncts > 0)
            {
                mapFuncts();
            }
            else
            {
                if (_shoutdownCount > 1) _shoutdownCount--;
                else if (_shoutdownCount == 1) break;
                else
                {
                    std::this_thread::sleep_for(std::chrono::seconds(1));
                }
            }
        }
    }
    
    void mapFuncts()
    {
        std::list<FuncData> cpyFuncDataVec;
        IdxType indexSize;
        
        //TODO rename in deletedIdxs
        std::map<IdxType, bool> cpyDeletedIdxs;
        
        _mutex.lock();
        
        //OPTIMIZATION?
        cpyFuncDataVec = _funcDataVec;
        indexSize = _size;
        
        _mutex.unlock();
        
        IdxType idx;
        
        //
        // defrag.
        //gather deleted idxs
        
        //TODO optimize?
        std::list<FuncData> cpyFuncDataVec2 = cpyFuncDataVec;
        
        IdxType dummyIdx=0;
        bool hasInsValue;
        YType yDummyVal;
        bool notNeeded = false;
        
        bool hasDelValue;
        IdxType delVal;
        
        size_t functDataSize = cpyFuncDataVec2.size();
        for (int i=0; i<functDataSize; i++)
        {
            //TODO check if ins and upd vals can be treated similarly.
            cpyFuncDataVec2.front().closure(dummyIdx, hasInsValue, yDummyVal, hasDelValue, delVal, notNeeded, yDummyVal);
            cpyFuncDataVec2.pop_front();
            
            if (hasDelValue)
            {
                if (cpyFuncDataVec2.size() > 0)
                {
                    idx = eval(delVal, cpyFuncDataVec2, hasInsValue, yDummyVal, true, notNeeded, yDummyVal);
                    
                    if (!hasInsValue)
                    {
                        cpyDeletedIdxs[idx] = true;
                    }
                }
                else
                {
                    cpyDeletedIdxs[delVal] = true;
                }
            }
        }
        //
        //
        
        hasInsValue=false;
        YType insYValue;
        bool hasUpdVal;
        YType yUpdValue;
        
        for (int i=0; i<indexSize; i++)
        {
            idx = eval(i, cpyFuncDataVec, hasInsValue, insYValue, true, hasUpdVal, yUpdValue);
         
            //!!! use update val if present
            if (hasInsValue)
            {
                IdxType insIdx = _yValMMapWrapper->size();
                
                _yValMutex.lock();
                _yValMMapWrapper->persistVal(insIdx, insYValue);
                _yValMutex.unlock();
                
                _doubleSyncedMMapWrapper.persist(i, insIdx);
            }
            else _doubleSyncedMMapWrapper.persist(i, idx);
        }
        
        _doubleSyncedMMapWrapper.shrinkSize(indexSize);
        
        
        //
        //start defrag.
        //
        assert(_yValMMapWrapper->size() - cpyDeletedIdxs.size() == indexSize);
        
        std::list<std::function<IdxType (IdxType inIdx)>> defragFuncts;
        
        
        IdxType defractSize = cpyDeletedIdxs.size();
        
        IdxType lidx;
        for (IdxType i=0; i<defractSize; i++)
        {
            lidx = indexSize+i;
            
            if (cpyDeletedIdxs.count(lidx) == 0)
            {
                //TODO remove.
                assert(cpyDeletedIdxs.size() > 0);
                
                IdxType cpyIdx = cpyDeletedIdxs.begin()->first;
                
                //TODO opt?
                cpyDeletedIdxs.erase(cpyIdx);
                
                YType cpyYVal = _yValMMapWrapper->getVal(lidx);
                
                _yValMutex.lock();
                _yValMMapWrapper->persistVal(cpyIdx, cpyYVal);
                _yValMutex.unlock();
                
                defragFuncts.push_back([lidx, cpyIdx] (IdxType inIdx) -> IdxType
                   {
                       IdxType retIdx = inIdx;
                
                       if (inIdx == lidx)
                       {
                           retIdx = cpyIdx;
                       }
                       
                       return retIdx;
                   }
                );
            }
            else
            {
                cpyDeletedIdxs.erase(lidx);
            }
        }
        
        
        for (int i=0; i<indexSize; i++)
        {
            idx = _doubleSyncedMMapWrapper.getBack(i);
            
            for (auto& func : defragFuncts)
            {
                idx = func(idx);
            }
            
            _doubleSyncedMMapWrapper.persist(i, idx);
        }
        //
        //end defrag.
        //
        
        
        _mutex.lock();
        
        auto begItr = _funcDataVec.begin();
        std::advance(begItr,_funcDataVec.size() - cpyFuncDataVec.size());
        _funcDataVec.erase(begItr, _funcDataVec.end());
        
        _doubleSyncedMMapWrapper.switchMMaps();
        
        //
        // defrag.
        //
        _yValMutex.lock();
        _yValMMapWrapper->shrinkSize(indexSize);
        _yValMutex.unlock();
        //
        //
        
        _mutex.unlock();
    }
    
    IdxType eval(IdxType idx, std::list<FuncData>& functDataList, bool& hasInsValue, YType& yInsValue, bool contEvalWhenUpdDetected, bool& hasUpdVal, YType& yUpdValue)
    {
        hasUpdVal = false;
        
        //not needed in this context.
        bool evalHasDelValue;
        bool evalHasUpdValue;
        
        IdxType delVal;
        IdxType updVal;
        
        for(auto& funcData : functDataList)
        {
            idx = funcData.closure(idx, hasInsValue, yInsValue, evalHasDelValue, delVal, evalHasUpdValue, updVal);
            
            if (hasInsValue)
            {
                break;
            }
            else if (evalHasUpdValue)
            {
                hasUpdVal = true;
                yUpdValue = updVal;
                
                if (!contEvalWhenUpdDetected) break;
            }
        }
        
        if (!hasInsValue) idx = _doubleSyncedMMapWrapper.get(idx);
        
        return idx;
    }
};

#endif
