//
//  DDTagedIndex.h
//  DynamicData
//
//  Created by mich2 on 9/21/12.
//  Copyright (c) 2012 -. All rights reserved.
//

#ifndef DynamicData_DDTagedIndex_h
#define DynamicData_DDTagedIndex_h
#include "DDIndex.h"

template<typename IdxType>
//TODO fix typo!
class DDTagedIndex
{
public:
    class DDTagHolder
    {
    public:
        
        
    private:
        DDTagedIndex<IdxType> _taggedIndex;
        
    };
    
private:
    class IdxYVal
    {
    public:
        unsigned long tag;
        IdxType idx;
        IdxType nextDiff;
    };

public:
    
    DDTagedIndex(size_t scopeVal, size_t idVal1, size_t idVal2, size_t idVal3) :
        _ddIndex(scopeVal, idVal1, idVal2, idVal3)
    {
        
    }
    
    DDTagedIndex(const DDTagedIndex&) = delete;
    const DDTagedIndex& operator=(const DDTagedIndex&) = delete;

    void tag(unsigned long tag, IdxType idx, IdxType size)
    {
        
    }

    void getTag(unsigned long tag)
    {
        
    }
    
    void getNextTag()
    {
        
    }
    
    void deleteTag(unsigned long tag)
    {
        
    }
    
    void insertIdx(IdxType idx)
    {
        
    }
    
    void deleteIdx(IdxType idx)
    {
        
    }
    
private:
    std::list<std::function<IdxType (IdxType inIdx)>> _funcDataVec;
    DDIndex<IdxType, IdxYVal> _ddIndex;
    
};

#endif