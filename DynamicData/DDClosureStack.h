//
//  DDClosureStack.h
//  DynamicData
//
//  Created by mich2 on 8/21/12.
//  Copyright (c) 2012 -. All rights reserved.
//

#ifndef DynamicData_DDClosureStack_h
#define DynamicData_DDClosureStack_h

/*
#include <vector>
#include "DDMMapAllocator.h"

template<typename IdxType>
class DDClosureStack
{
private:
    
    enum ClosureType
    {
        SumClosure,
        InsertClosure,
        DeleteClosure
    };
    
    class ClosureParams
    {
    public:
        ClosureType type;
        size_t idVal;
        IdxType arg1;
        IdxType arg2;
    };
    
public:
    
    DDClosureStack(size_t scopeVal, size_t idVal, unsigned int stackSize) :
        _persStack(DDMMapAllocator<unsigned int>::SHARED()->template getHandleFromDataStore<ClosureParams>(scopeVal, idVal)),
        _stackSize(stackSize)
    {
        loadClosureCreatorMap();
        loadClosures();
    }
    
    DDClosureStack(const DDClosureStack&) = delete;
    const DDClosureStack& operator=(const DDClosureStack&) = delete;

    
    void addSumClosure(size_t idVal, IdxType base, IdxType delta)
    {
        auto closure = createSumClosureImp(base, delta);
        
        ClosureParams params;
        params.type = SumClosure;
        params.idVal = idVal;
        params.arg1 = base;
        params.arg2 = delta;
        
        addClosure(closure, params);
    }
    
    void addInsertClosure(size_t idVal, IdxType insertIdx)
    {
        auto closure = createInsertClosureImp(insertIdx, -1);
    
        ClosureParams params;
        params.type = InsertClosure;
        params.idVal = idVal;
        params.arg1 = insertIdx;
        params.arg2 = -1;
        
        addClosure(closure, params);
    }
    
    void addDeleteClosure(size_t idVal, IdxType deleteIdx)
    {
        auto closure = createDeleteClosureImp(deleteIdx, -1);
        
        ClosureParams params;
        params.type = DeleteClosure;
        params.idVal = idVal;
        params.arg1 = deleteIdx;
        params.arg2 = -1;
        
        addClosure(closure, params);
    }
    
private:
    std::unique_ptr<MMapWrapper<unsigned int, ClosureParams>> _persStack;
    
    typedef double (DDClosureStack::*ClosureCreator)();
    std::map<ClosureType, ClosureCreator> _closureCreatorMap;
    
    std::vector<std::function<IdxType (IdxType inIdx)>> _closures;
    unsigned int _stackSize;
    
    void loadClosureCreatorMap()
    {
        _closureCreatorMap[SumClosure]=&DDClosureStack::createSumClosureImp;
        _closureCreatorMap[InsertClosure]=&DDClosureStack::createInsertClosureImp;
        _closureCreatorMap[DeleteClosure]=&DDClosureStack::createDeleteClosureImp;
    }

    std::function<IdxType (IdxType inIdx)> createSumClosureImp(IdxType base, IdxType delta)
    {
        return [] (IdxType inIdx) -> IdxType {  };
    }
    
    std::function<IdxType (IdxType inIdx)> createInsertClosureImp(IdxType insertIdx, IdxType notUsed)
    {
        return [] (IdxType inIdx) -> IdxType {  };
    }
    
    std::function<IdxType (IdxType inIdx)> createDeleteClosureImp(IdxType deleteIdx, IdxType notUsed)
    {
        return [] (IdxType inIdx) -> IdxType {  };
    }
    
    void loadClosures()
    {
        for (unsigned int i=0; i<_persStack->size(); i++)
        {
            ClosureParams closureParams = _persStack->getVal(i);
            
            auto closure = this.*_closureCreatorMap[closureParams.type](closureParams.arg1, closureParams.arg2);
            _closures.push_back(closure);
        }
    }
    
    void saveClosure(ClosureParams closureParams)
    {
        IdxType insertPoint = _persStack->size();
        _persStack->persistVal(insertPoint, closureParams);
    }
    
    void addClosure(std::function<IdxType (IdxType inIdx)> closure, ClosureParams params)
    {
        _closures.push_back(closure);
        saveClosure(params);
    }
    
};
*/

#endif
