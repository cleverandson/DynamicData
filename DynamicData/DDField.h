//
//  DDField.h
//  DynamicData
//
//  Created by mich2 on 10/31/12.
//  Copyright (c) 2012 -. All rights reserved.
//

#ifndef DynamicData_DDField_h
#define DynamicData_DDField_h

#include <utility>

#include "DDInsertField.h"
#include "DDDeleteField.h"

template<typename IdxType, class CachedElement>
class DDField
{
public:
    
    DDField() : _fieldSize(0) {}
    
    DDField(DDField<IdxType, CachedElement>&& other) :
        _insertField(std::forward<DDInsertField<IdxType, CachedElement>>(other._insertField)),
        _deleteField(std::forward<DDDeleteField<IdxType>>(other._deleteField)),
        _fieldSize(other._fieldSize)
    {}
    
    //TODO implement.
    //DDField& operator=(DDField &&) = default;
    
    void operator=(DDField<IdxType, CachedElement>&& rhs)
    {
        _insertField = std::forward<DDInsertField<IdxType, CachedElement>>(rhs._insertField);
        _deleteField = std::forward<DDDeleteField<IdxType>>(rhs._deleteField);
        _fieldSize = rhs._fieldSize;
    }
    
    DDField(const DDField&) = delete;
    const DDField& operator=(const DDField&) = delete;

    void insertIdx(IdxType idx, const CachedElement& cachedElement)
    {
        idx = _deleteField.adjustFieldAndEval(idx);
        
        idx++;
        _insertField.addIdx(idx, cachedElement);
        
        _fieldSize++;
    }
    
    void deleteIdx(IdxType idx)
    {
        _deleteField.addIdx(idx);
        
        _fieldSize++;
    }
    
    IdxType eval(IdxType idx, bool& hasCacheElement, CachedElement& cachedElement)
    {
        if (_fieldSize > 0)
        {
            idx = _deleteField.eval(idx);
            idx = _insertField.eval(idx, hasCacheElement, cachedElement);
        }
    
        return idx;
    }
    
    void startItr()
    {
        _insertField.fieldItr.startItr();
        _deleteField.fieldItr.startItr();
    }
    
    IdxType deleteFieldItrEvalAndStep()
    {
        return _deleteField.fieldItr.itrEvalAndStep();
    }
    
    std::vector<IdxType> deleteFieldAllDeleteIdxs()
    {
        return _deleteField.allDeleteIdxs();
    }
    
    IdxType insertFieldEval(IdxType idx, bool& hasCacheElement, CachedElement& cachedElement)
    {
        return _insertField.fieldItr.itrEval(idx, hasCacheElement, cachedElement);
    }
    
    void clear()
    {
        _insertField.clear();
        _deleteField.clear();
        _fieldSize = 0;
    }
    
    size_t size()
    {
        return _fieldSize;
    }
    
private:
    DDInsertField<IdxType, CachedElement> _insertField;
    DDDeleteField<IdxType> _deleteField;
    size_t _fieldSize;
};


#endif
