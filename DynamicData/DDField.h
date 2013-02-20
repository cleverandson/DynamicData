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
