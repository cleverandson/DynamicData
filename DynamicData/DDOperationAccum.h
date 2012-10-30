//
//  DDOperationAccum.h
//  DynamicData
//
//  Created by mich2 on 10/29/12.
//  Copyright (c) 2012 -. All rights reserved.
//

#ifndef DynamicData_DDOperationAccum_h
#define DynamicData_DDOperationAccum_h

/*
template<typename IdxType, class CachedElement>
class DDOperationAccum
{
public:
    
    DDOperationAccum() :
        _monoDel(1),
        _monoIns(-1)
    {}
    
    DDOperationAccum(const DDOperationAccum&) = delete;
    const DDOperationAccum& operator=(const DDOperationAccum&) = delete;
    
    void insertIdx(IdxType idx, const CachedElement& cachedElement)
    {
        idx = _monoDel.eval(idx);

        _monoIns.addIdx(idx, cachedElement);
    }
    
    void deleteIdx(IdxType idx)
    {
        _monoDel.addIdx(idx);
    }
    
    void eval(IdxType idx, bool& hasCacheElement, CachedElement& cachedElement)
    {
        _monoIns.eval(_monoDel.eval(idx), hasCacheElement, cachedElement);
    }
    
    void clear()
    {
        _monoIns.clear();
        _monoDel.clear();
    }
    
private:
    DDMonotonic<IdxType, IdxType> _monoDel;
    DDMonotonic<IdxType, CachedElement> _monoIns;
}; 
*/

#endif
