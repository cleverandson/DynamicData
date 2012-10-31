//
//  DDField.h
//  DynamicData
//
//  Created by mich2 on 10/31/12.
//  Copyright (c) 2012 -. All rights reserved.
//

#ifndef DynamicData_DDField_h
#define DynamicData_DDField_h

#include "DDInsertField.h"
#include "DDDeleteField.h"

template<typename IdxType, class CachedElement>
class DDField
{
public:
    
    DDField() {}
    
    DDField(const DDField&) = delete;
    const DDField& operator=(const DDField&) = delete;

    void insertIdx(IdxType idx, const CachedElement& cachedElement)
    {
        idx++;
        
        idx = _deleteField.eval(idx);
        _insertField.addIdx(idx, cachedElement);
    }
    
    void deleteIdx(IdxType idx)
    {
        _deleteField.addIdx(idx);
    }
    
    IdxType eval(IdxType idx, bool& hasCacheElement, CachedElement& cachedElement)
    {
        idx = _deleteField.eval(idx);
        return _insertField.eval(idx, hasCacheElement, cachedElement);
    }
    
    void clear()
    {
        _insertField.clear();
        _deleteField.clear();
    }
    
private:
    DDInsertField<IdxType, CachedElement> _insertField;
    DDDeleteField<IdxType> _deleteField;
};


#endif
