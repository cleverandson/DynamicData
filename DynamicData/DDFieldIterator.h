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

#ifndef DynamicData_DDFieldIterator_h
#define DynamicData_DDFieldIterator_h

//TODO make two classes for the two fields.
template<typename IdxType, class Field, class CachedElement>
class DDFieldIterator
{
public:
    
    DDFieldIterator(Field& field) :
        _field(field),
        _currIdx(0)
    {}
    
    virtual ~DDFieldIterator() {}
    
    DDFieldIterator(const DDFieldIterator&) = delete;
    const DDFieldIterator& operator=(const DDFieldIterator&) = delete;
    
    
    void startItr()
    {
        _boundaryItr = _field.beginItr();
        _currIdx = 0;
    }
    
    IdxType itrEvalAndStep()
    {
        IdxType res = _field.eval(_currIdx, _boundaryItr);
        _currIdx++;
        
        return res;
    }
    
    /*
    //TODO remove?
    IdxType itrEvalAndStep(bool& hasCacheElement, CachedElement& cachedElement)
    {
        IdxType res = _field.eval(_currIdx, _boundaryItr, hasCacheElement, cachedElement);
        _currIdx++;
        
        return res;
    }
    */
    
    //TODO check this!
    IdxType itrEval(IdxType idx, bool& hasCacheElement, CachedElement& cachedElement)
    {
        if (idx < _currIdx) assert(0);
        _currIdx = idx;
        
        IdxType res = _field.eval(_currIdx, _boundaryItr, hasCacheElement, cachedElement);
        //_currIdx++;
        
        return res;
    }
    
    void reset()
    {
        _currIdx = 0;
    }
    
private:
    Field& _field;
    IdxType _currIdx;
    typename Field::BoundItr _boundaryItr;
};

#endif
