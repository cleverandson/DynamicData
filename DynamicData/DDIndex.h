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
        
        DoubleSyncedMMapWrapper(DoubleSyncedMMapWrapper&& other) :
            _mmapWrapper1(std::forward<MMapWrapperPtr<IdxType, IdxType, MMapHeader>>(other._mmapWrapper1)),
            _mmapWrapper2(std::forward<MMapWrapperPtr<IdxType, IdxType, MMapHeader>>(other._mmapWrapper2)),
            _activeMapIdx(other._activeMapIdx)
        {}
        
        void operator=(DoubleSyncedMMapWrapper&& rhs)
        {
            _mmapWrapper1.swap(rhs._mmapWrapper1);
            _mmapWrapper2.swap(rhs._mmapWrapper2);
            _activeMapIdx = rhs._activeMapIdx;
        }
        
        DoubleSyncedMMapWrapper(const DoubleSyncedMMapWrapper&) = delete;
        const DoubleSyncedMMapWrapper& operator=(const DoubleSyncedMMapWrapper&) = delete;
        
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
        
        void unpersist()
        {
            _mmapWrapper1->unpersist();
            _mmapWrapper2->unpersist();
            
            _mmapWrapper1.reset();
            _mmapWrapper2.reset();
        }
        
    private:
        MMapWrapperPtr<IdxType, IdxType, MMapHeader> _mmapWrapper1;
        MMapWrapperPtr<IdxType, IdxType, MMapHeader> _mmapWrapper2;
        unsigned int _activeMapIdx;
    };
    
    class YValMapHeader { };
    
public:
    
    DDIndex(size_t scopeVal, size_t idVal1, size_t idVal2, size_t idVal3) :
        _doubleSyncedMMapWrapper(scopeVal, idVal1, idVal2),
        _yValMMapWrapper(DDMMapAllocator<IdxType>::SHARED()->template getHandleFromDataStore<YType, YValMapHeader>(scopeVal, idVal3)),
        _size(_doubleSyncedMMapWrapper.size()),
        _shoutdownCount(0),
        _activPassivField(DDField<IdxType, YType>(), DDField<IdxType, YType>()),
        _reduceAndSwapThread(&DDIndex::reduceAndSwapMap,this)
    {}
    
    DDIndex(DDIndex&& other) :
        _doubleSyncedMMapWrapper(std::forward<DoubleSyncedMMapWrapper>(other._doubleSyncedMMapWrapper)),
        _yValMMapWrapper(std::forward<MMapWrapperPtr<IdxType, YType, YValMapHeader>>(other._yValMMapWrapper)),
        _size(other._size),
        _shoutdownCount(other._shoutdownCount.fetch_add(0)),
        _activPassivField(std::forward<DDActivePassivePtr<DDField<IdxType, YType>>>(other._activPassivField)),
        _reduceAndSwapThread(std::thread(&DDIndex::reduceAndSwapMap,this))
    {}
    
    void operator=(DDIndex&& rhs)
    {
        finish();
        rhs.finish();
     
        _doubleSyncedMMapWrapper = std::forward<DoubleSyncedMMapWrapper>(rhs._doubleSyncedMMapWrapper);
        _yValMMapWrapper.swap(rhs._yValMMapWrapper);
        _size = rhs._size;
        _shoutdownCount = rhs._shoutdownCount.fetch_add(0);
        
        //TODO remove this
        assert(_shoutdownCount == rhs._shoutdownCount);
        
        _activPassivField = std::forward<DDActivePassivePtr<DDField<IdxType, YType>>>(rhs._activPassivField);
        _reduceAndSwapThread = std::thread(&DDIndex::reduceAndSwapMap,this);
    }
    
    //TODO make private.
    void finish()
    {
        if (_reduceAndSwapThread.joinable())
        {
            _mutex.lock();
            _mutex.unlock();
            
            _shoutdownCount = 2;
            
            _reduceAndSwapThread.join();
            
            _shoutdownCount = 0;
        }
    }
    
    ~DDIndex()
    {
        finish();
    }
    
    void unpersist()
    {
        finish();
        
        _doubleSyncedMMapWrapper.unpersist();
        
        _yValMMapWrapper->unpersist();
        _yValMMapWrapper.reset();
    }
    
    DDIndex(const DDIndex&) = delete;
    const DDIndex& operator=(const DDIndex&) = delete;
    
    YType get(IdxType idx)
    {
        YType yVal;
        
        assert(idx < _size);
        
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
                    IdxType idx2 = _doubleSyncedMMapWrapper.get(idx);
                 
                    {
                        std::unique_lock<std::mutex> lock(_yValMutex);
                        yVal = _yValMMapWrapper->getVal(idx2);
                    }
                }
            }

            _mutex.unlock();
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
    MMapWrapperPtr<IdxType, YType, YValMapHeader> _yValMMapWrapper;
    std::mutex _yValMutex;
    
    IdxType _size;
    
    std::mutex _mutex;
    
    std::atomic<int> _shoutdownCount;
    
    DDActivePassivePtr<DDField<IdxType, YType>> _activPassivField;
    
    std::thread _reduceAndSwapThread;
    
    
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
        
        
        backField.startItr();
        std::vector<IdxType> deletedIdxs2;
        std::vector<IdxType> remapIdxs;
        
        auto reduceMapOntoDoubleSyncedMMapWrapper = [this, &backField, &deletedIdxs2, &indexSize, &remapIdxs] (IdxType idxIN, IdxType range)
        {
            bool hasCacheElement;
            YType yObj;
            
            for (IdxType i = 0; i < range; i++)
            {
                IdxType idx = i + idxIN;
                
                //TODO remove tuple.
                IdxType ret = backField.deleteFieldItrEvalAndStep();
                
                IdxType mappedFieldidx = backField.insertFieldEval(ret, hasCacheElement, yObj);
                
                if (!hasCacheElement)
                {
                    IdxType mappedIdx = _doubleSyncedMMapWrapper.get(mappedFieldidx);
                    
                    //remap idxs which are too big.
                    if (mappedIdx >= indexSize)
                    {
                        remapIdxs.push_back(idx);
                    }
                    
                    _doubleSyncedMMapWrapper.persist(idx, mappedIdx);
                }
                else
                {
                    IdxType nextIdx;
                    
                    std::unique_lock<std::mutex> lock(_yValMutex);
                    
                    nextIdx = _yValMMapWrapper->size();
                    _yValMMapWrapper->persistVal(nextIdx, yObj);
                     
                    if (nextIdx >= indexSize)
                    {
                        remapIdxs.push_back(idx);
                    }
                    
                    _doubleSyncedMMapWrapper.persist(idx, nextIdx);
                }
            }
        };
        
        rangeLoop(range, indexSize, reduceMapOntoDoubleSyncedMMapWrapper);
        
        
        
        backField.startItr();
        
        //collect idxs to delete.
        std::vector<IdxType> delIdxs = backField.deleteFieldAllDeleteIdxs();
        
        for (auto itr = delIdxs.begin(); itr<delIdxs.end(); itr++)
        {
            bool hasCacheElement;
            YType yObj;
            
            IdxType idx = backField.insertFieldEval(*itr, hasCacheElement, yObj);
            
            if (!hasCacheElement)
            {
                IdxType mappedIdx = _doubleSyncedMMapWrapper.get(idx);
                
                if (mappedIdx < indexSize)
                {
                    deletedIdxs2.push_back(mappedIdx);
                }
            }
        }
        
        //close gaps in YVal Map.
        IdxType idx;
        IdxType mvidx;
        IdxType delIdx;
        YType yObj;
        
        //TODO check what happens if indexSize == 0!!
        IdxType tailIdx = indexSize - 1;
        
        IdxType remapIdx = 0;
        
        assert(deletedIdxs2.size() >= remapIdxs.size());
        
        for (IdxType i = 0; i< deletedIdxs2.size() > 0; i++)
        {
            std::unique_lock<std::mutex> lock(_yValMutex);
            
            if (remapIdxs.size() > remapIdx)
            {
                idx = remapIdxs[remapIdx];
                remapIdx++;
            }
            else
            {
                idx = tailIdx;
                tailIdx--;
            }
            
            
            mvidx = _doubleSyncedMMapWrapper.getBack(idx);
            
            yObj = _yValMMapWrapper->getVal(mvidx);
            
            delIdx = deletedIdxs2[i];
            
            _yValMMapWrapper->persistVal(delIdx, yObj);
            _doubleSyncedMMapWrapper.persist(idx, delIdx);
        }
        
        
        _mutex.lock();
        
        backField.clear();
        
        
        _doubleSyncedMMapWrapper.resize(indexSize);
        _doubleSyncedMMapWrapper.switchMMaps();
        
        
        {
            std::unique_lock<std::mutex> lock(_yValMutex);
            _yValMMapWrapper->resize(indexSize);
        }
        
        _mutex.unlock();
    }
};

#endif
