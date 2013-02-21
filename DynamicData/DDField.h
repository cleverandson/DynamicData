/*

    Copyright (c) 2013, Clever & Son
    All rights reserved.

    Redistribution and use in source and binary forms, with or without modification, are
    permitted provided that the following conditions are met:

    Redistributions of source code must retain the above copyright notice, this list of
    conditions and the following disclaimer.
    Redistributions in binary form must reproduce the above copyright notice, this list of
    conditions and the following disclaimer in the documentation and/or other materials
    provided with the distribution.
    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND
    CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
    INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
    MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
    CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
    SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
    NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
    CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
    STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
    ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
    ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/

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
        else hasCacheElement = false;
    
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
