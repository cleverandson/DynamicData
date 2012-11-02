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
#include <vector>

#include "DDMMapAllocator.h"
#include "DDActivePassivePtr.h"
#include "DDField.h"

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
        
        IdxType backSize()
        {
            IdxType size;
            
            if (_activeMapIdx == 1 || _activeMapIdx == 0)
            {
                size = _mmapWrapper2->size();
            }
            else
            {
                size = _mmapWrapper1->size();
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
        
        void resize(IdxType size)
        {
            if (_activeMapIdx == 0 || _activeMapIdx == 1)
            {
                _mmapWrapper2->resize(size);
            }
            else
            {
                _mmapWrapper1->resize(size);
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
    
    //TODO should be removed?
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
        _reduceAndSwapThread(&DDIndex::reduceAndSwapMap,this),
    
        _activPassivField(DDField<IdxType, YType>(), DDField<IdxType, YType>())
    {}
    
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
        
        if (idx < _size)
        {
            _mutex.lock();
            
            bool hasCacheElement;
            idx = _activPassivField->eval(idx, hasCacheElement, yVal);
            
            if (!hasCacheElement)
            {
                idx = _activPassivField.back().eval(idx, hasCacheElement, yVal);
                
                if (!hasCacheElement)
                {
                    idx = _doubleSyncedMMapWrapper.get(idx);
                    yVal = _yValMMapWrapper->getVal(idx);
                }
            }

            _mutex.unlock();
            
            succeeded = true;
        }
        
        return yVal;
    }
    
    /*
    void updateIdx(IdxType idx, YType yValue)
    {
        if (!_shoutdownCount)
        {
            assert(idx < _size);
            
            _mutex.lock();
            
            FuncData funcData;
            
            funcData.closure = [idx, yValue] (IdxType inIdx, bool& hasInsValue, YType& insVal, bool& hasDelValue, IdxType& delVal, bool& hasUpdVal, YType& updVal) -> IdxType
            {
                hasInsValue = false;
                hasDelValue = false;
                hasUpdVal = false;
                
                if (inIdx == idx)
                {
                    updVal = yValue;
                    hasUpdVal = true;
                }
                
                return inIdx;
            };
            
            _funcDataVec.push_front(funcData);
            
            _mutex.unlock();
        }
    }
    */
    
    void insertIdx(IdxType idx, YType yValue)
    {
        if (!_shoutdownCount)
        {
            assert(idx < _size + 1);
            
            _mutex.lock();
            
            _activPassivField->insertIdx(idx, yValue);
            
            _size++;
            
            _mutex.unlock();
        }
    }
    
    void deleteIdx(IdxType idx)
    {
        if (!_shoutdownCount)
        {
            assert(idx < _size);
            
            _mutex.lock();
         
            _activPassivField->deleteIdx(idx);
            
            _size--;
            
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
    
    IdxType _size;
    
    std::mutex _mutex;
    std::atomic<int> _shoutdownCount;
    
    std::thread _reduceAndSwapThread;
    
    DDActivePassivePtr<DDField<IdxType, YType>> _activPassivField;
    
    void reduceAndSwapMap()
    {
        //return;
        
        while(true)
        {
            size_t fieldSize;
            
            _mutex.lock();
            
            fieldSize = _activPassivField->size();
            
            _mutex.unlock();
            
            if (fieldSize > 0)
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
        IdxType indexSize;
        
        
        _mutex.lock();
        
        _activPassivField.swap();
        DDField<IdxType, YType>& backField = _activPassivField.back();
        
        indexSize = _size;
        
        _mutex.unlock();
        
        
        //TODO test.
        IdxType range = 200;
        
        auto rangeLoop = [] (IdxType range, IdxType size, std::function<void (IdxType idx, IdxType range)> closure)
        {
            IdxType currIdx = 0;
            bool done = false;
            while (true)
            {
                if (currIdx + range >= size)
                {
                    done = true;
                    range = size - currIdx;
                }
                
                closure(currIdx, range);
                
                //exit
                if (done) break;
                
                currIdx += range;
            }
        };
        
    
        //xxx
        backField.startItr();
        std::vector<IdxType> deletedIdxs2;
        size_t insCount = 0;
        
        auto reduceMapOntoDoubleSyncedMMapWrapper = [this, &backField, &deletedIdxs2, &insCount] (IdxType idxIN, IdxType range)
        {
            bool hasCacheElement;
            YType yObj;
            
            for (IdxType i = 0; i < range; i++)
            {
                std::tuple<IdxType, bool, IdxType> ret = backField.deleteFieldItrEvalAndStep();
                
                //eval deleted idx.
                if (std::get<1>(ret))
                {
                    IdxType idx = backField.insertFieldEval(std::get<2>(ret), hasCacheElement, yObj);

                    if (!hasCacheElement)
                    {
                        deletedIdxs2.push_back(_doubleSyncedMMapWrapper.get(idx));
                    }
                }
                
                IdxType idx = backField.insertFieldEval(std::get<0>(ret), hasCacheElement, yObj);
                
                if (!hasCacheElement)
                {
                    IdxType mapedIdx = _doubleSyncedMMapWrapper.get(idx);
                    
                    _doubleSyncedMMapWrapper.persist(i + idxIN, mapedIdx);
                }
                else
                {
                    IdxType nextIdx;
                    
                    if (deletedIdxs2.size() == 0)
                    {
                        nextIdx = _yValMMapWrapper->size();
                        _yValMMapWrapper->persistVal(nextIdx, yObj);
                    }
                    else
                    {
                        nextIdx = deletedIdxs2.back();
                        deletedIdxs2.pop_back();
                    }
                    
                    _doubleSyncedMMapWrapper.persist(i + idxIN, nextIdx);
                    insCount++;
                }
            }
        };
        
        rangeLoop(range, indexSize, reduceMapOntoDoubleSyncedMMapWrapper);
        
        //close gaps in YVal Map.
        IdxType idx;
        IdxType mvidx;
        IdxType delIdx;
        YType yObj;
        
        for (int i = 0; i< deletedIdxs2.size() > 0; i++)
        {
            idx = indexSize - i - 1;
            
            mvidx = _doubleSyncedMMapWrapper.getBack(idx);
            
            yObj = _yValMMapWrapper->getVal(mvidx);
            
            delIdx = deletedIdxs2[i];
            
            _yValMMapWrapper->persistVal(delIdx, yObj);
            _doubleSyncedMMapWrapper.persist(idx, delIdx);
        }
        
        
        _mutex.lock();
        
        backField.clear();
        
        _doubleSyncedMMapWrapper.switchMMaps();
        
        _yValMMapWrapper->resize(indexSize);
        
        _mutex.unlock();
    }
};

#endif
