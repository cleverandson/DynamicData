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
    
    class FuncData
    {
    public:
        std::function<IdxType (IdxType inIdx, bool& hasInsValue, YType& insVal, bool& hasDelValue, IdxType& delVal, bool& hasUpdVal, YType& updVal)> closure;
    };
    
    class YValMapHeader { };
    
   
    class ReduceYValue
    {
    public:
        
        ReduceYValue() :
            idx(0),
            terminated(false),
            hasUpdVal(false),
            hasInsVal(false)
        {
            
        }
        
        void reset()
        {
            idx = 0;
            terminated = false;
            hasUpdVal = false;
            hasInsVal = false;
        }
        
        IdxType idx;
        bool terminated;
        bool hasUpdVal;
        IdxType updIdx;
        bool hasInsVal;
        IdxType insIdx;
    };
    
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
        //TODO rename in cpyFuncDataList.
        std::list<FuncData> cpyFuncDataVec;
        size_t cpyFuncDataVecSize;
        IdxType indexSize;
        
        
        _mutex.lock();
        
        //TODO limit number of proccessed closures.
        //OPTIMIZATION?
        cpyFuncDataVec = _funcDataVec;
        cpyFuncDataVecSize = cpyFuncDataVec.size();
        
        indexSize = _size;
        
        _mutex.unlock();
        
        std::vector<ReduceYValue> deletedIdxs;
        std::vector<YType> updVals;
        std::vector<std::pair<IdxType, YType>> insVals;
        
        IdxType range = 200;
        std::vector<IdxType> remapIdxs;
        
        
        auto reduceMapClosure = [] (IdxType startIdx, IdxType range, bool first, FuncData& funcData, std::vector<ReduceYValue>& tempMap, std::vector<YType>& updVals, std::vector<std::pair<IdxType, YType>>& insVals)
        {
            ReduceYValue ry;
            
            IdxType idx;
            for (IdxType k=0; k<range; k++)
            {
                idx = k + startIdx;
                
                if (first)
                {
                    ry.reset();
                    ry.idx = idx;
                }
                else
                {
                    ry = tempMap[k];
                }
                
                if (!ry.terminated)
                {
                    bool hasInsValue2;
                    YType insVal2;
                    bool hasUpdValue2;
                    YType updVal2;
                    bool hasDelValue2;
                    IdxType delVal2;
                    
                    ry.idx = funcData.closure(ry.idx, hasInsValue2, insVal2, hasDelValue2, delVal2, hasUpdValue2, updVal2);
                    
                    if (hasUpdValue2)
                    {
                        ry.hasUpdVal = true;
                        ry.updIdx = updVals.size();
                        updVals.push_back(updVal2);
                    }
                    else if (hasInsValue2)
                    {
                        YType& yVal = insVal2;
                        
                        if (ry.hasUpdVal)
                        {
                            yVal = updVals[ry.updIdx];
                            ry.hasUpdVal = false;
                        }
                        
                        ry.hasInsVal = true;
                        ry.insIdx = insVals.size();
                        ry.terminated = true;
                        
                        insVals.push_back(std::make_pair(0, yVal));
                    }
                    
                    tempMap[k] = ry;
                }
            }
        };
        
        auto reduceDeletedIdxs = [&deletedIdxs] (FuncData& funcData)
        {
            bool hasInsValue2;
            YType insVal2;
            bool hasUpdValue2;
            YType updVal2;
            bool hasDelValue2;
            IdxType delVal2;
            
            //
            // reduced deleted idxs.
            for (IdxType j=0; j<deletedIdxs.size(); j++)
            {
                ReduceYValue& ry = deletedIdxs[j];
                
                if (!ry.terminated)
                {
                    ry.idx = funcData.closure(ry.idx, hasInsValue2, insVal2, hasDelValue2, delVal2, hasUpdValue2, updVal2);
                    
                    //TODO check!
                    if (hasInsValue2)
                    {
                        ry.terminated = true;
                    }
                }
            }
            
            //check if del idx exists.
            IdxType idx=0;
            funcData.closure(idx, hasInsValue2, insVal2, hasDelValue2, delVal2, hasUpdValue2, updVal2);
            
            if (hasDelValue2)
            {
                ReduceYValue delY;
                delY.idx = delVal2;
                deletedIdxs.push_back(delY);
            }
            //
            //
        };
        
        
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
        
        auto reduceMapOntoDoubleSyncedMMapWrapper = [this, &cpyFuncDataVec, &insVals, &updVals, &remapIdxs, indexSize, &reduceMapClosure] (IdxType idxIN, IdxType range)
        {
            std::vector<ReduceYValue> tempMap(range);
            
            bool isFirstClosure = true;
            auto itr = cpyFuncDataVec.begin();
            for (itr=cpyFuncDataVec.begin() ; itr != cpyFuncDataVec.end(); itr++)
            {
                reduceMapClosure(idxIN, range, isFirstClosure, *itr, tempMap, updVals, insVals);
                
                isFirstClosure = false;
            }
            
            
            //map cached values.
            for (IdxType k=0; k<range; k++)
            {
                ReduceYValue& refRy = tempMap[k];
                
                if (!refRy.terminated)
                {
                    refRy.idx = _doubleSyncedMMapWrapper.get(refRy.idx);
                }
            }
            
            //
            // copy the reduced idxs onto _doubleSyncedMMapWrapper
            _doubleSyncedMMapWrapper.resize(_doubleSyncedMMapWrapper.backSize() + range);
            ReduceYValue ry;
            IdxType idx;
            for (IdxType k=0; k<range; k++)
            {
                idx = idxIN+k;
                
                ry = tempMap[k];
                
                if (ry.hasInsVal)
                {
                    insVals[ry.insIdx].first = idx;
                }
                else
                {
                    _doubleSyncedMMapWrapper.persist(idx, ry.idx);
                    
                    if (ry.idx >= indexSize)
                    {
                        remapIdxs.push_back(idx);
                    }
                }
            }
        };
        
        
        //
        //reduce to double map
        //
        rangeLoop(range, indexSize, reduceMapOntoDoubleSyncedMMapWrapper);
        
        //
        //reduce deleted idxs and push them in deltedIdxs
        //
        auto itr = cpyFuncDataVec.begin();
        for (itr=cpyFuncDataVec.begin() ; itr != cpyFuncDataVec.end(); itr++)
        {
            reduceDeletedIdxs(*itr);
        }
        
        auto processDeletedIdxs = [&deletedIdxs, indexSize] (IdxType range, std::function<void (IdxType idx, bool hasDelIdx, IdxType delIdx)> closure)
        {
            IdxType delVecIdx=0;
            IdxType delIdx;
            for (IdxType i=0; i<range; i++)
            {
                bool hasDelIdx=false;
                if (delVecIdx < deletedIdxs.size())
                {
                    while (true)
                    {
                        ReduceYValue& delRy = deletedIdxs[delVecIdx];
                        delVecIdx++;
                        
                        if (!delRy.terminated && delRy.idx < indexSize)
                        {
                            delIdx = delRy.idx;
                            delRy.terminated = true;
                            
                            hasDelIdx = true;
                            break;
                        }
                        
                        if (delVecIdx >= deletedIdxs.size())
                        {
                            break;
                        }
                    }
                }
                
                closure(i, hasDelIdx, delIdx);
            }
        };
        
        auto moveRemapIdxs = [this, &remapIdxs](IdxType idx, bool hasDelIdx, IdxType delIdx)
        {
            assert(hasDelIdx);
            
            IdxType mappedIdx = _doubleSyncedMMapWrapper.getBack(remapIdxs[idx]);
            YType yv = _yValMMapWrapper->getVal(mappedIdx);
            
            _doubleSyncedMMapWrapper.persist(remapIdxs[idx], delIdx);
            
            _yValMutex.lock();
            _yValMMapWrapper->persistVal(delIdx, yv);
            _yValMutex.unlock();
        };
        
        auto insertInsVals = [this, &insVals](IdxType idx, bool hasDelIdx, IdxType delIdx)
        {
            std::pair<IdxType, YType> insVal = insVals[idx];
            
            if (!hasDelIdx) delIdx = _yValMMapWrapper->size();
                
            _doubleSyncedMMapWrapper.persist(insVal.first, delIdx);
            
            _yValMutex.lock();
            _yValMMapWrapper->persistVal(delIdx, insVal.second);
            _yValMutex.unlock();
        };
        
        //
        //move the remap idxs.
        //
        processDeletedIdxs(remapIdxs.size(), moveRemapIdxs);
        
        
        //
        //insert the YValues
        //
        processDeletedIdxs(insVals.size(), insertInsVals);
        
        _mutex.lock();
        
        auto begItr = _funcDataVec.begin();
        std::advance(begItr,_funcDataVec.size() - cpyFuncDataVecSize);
        _funcDataVec.erase(begItr, _funcDataVec.end());
        
        _doubleSyncedMMapWrapper.switchMMaps();
        
        _yValMutex.lock();
        _yValMMapWrapper->resize(indexSize);
        _yValMutex.unlock();
        
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
        
        //OPT possible optimization here?
        bool aborted = false;
        for(auto& funcData : functDataList)
        {
            idx = funcData.closure(idx, hasInsValue, yInsValue, evalHasDelValue, delVal, evalHasUpdValue, updVal);
            
            if (hasInsValue)
            {
                aborted = true;
                break;
            }
            else if (!hasUpdVal && evalHasUpdValue)
            {
                hasUpdVal = true;
                yUpdValue = updVal;
                
                if (!contEvalWhenUpdDetected)
                {
                    aborted = true;
                    break;
                }
            }
        }
        
        if (!aborted)
        {
            idx = _doubleSyncedMMapWrapper.get(idx);
        }
        
        return idx;
    }
};

#endif
