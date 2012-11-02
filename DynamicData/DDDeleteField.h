//
//  DDDeleteField.h
//  DynamicData
//
//  Created by mich2 on 10/30/12.
//  Copyright (c) 2012 -. All rights reserved.
//

#ifndef DynamicData_DDDeleteField_h
#define DynamicData_DDDeleteField_h

#include <set>
#include "DDFieldIterator.h"

template<typename IdxType>
class DDDeleteField
{
private:
    
    class Dummy{};
    
    class CompoundElement
    {
    public:
        
        CompoundElement(IdxType idxIN) :
        idx(idxIN),
        diff(1)
        { }
        
        IdxType idx;
        mutable IdxType diff;
    };
    
    class Comperator
    {
        public:

        bool operator() (const CompoundElement& lhs, const CompoundElement& rhs) const
        {
            return lhs.idx < rhs.idx;
        }
    };
    
    typedef std::set<CompoundElement, Comperator> SetType;
    
            
public:
    
    DDDeleteField() :
        fieldItr(*this)
    {}
    
    DDDeleteField(DDDeleteField&& other) :
        _set(std::forward<std::set<CompoundElement, Comperator>>(other._set)),
        fieldItr(*this)
    {}
            
    void operator=(DDDeleteField<IdxType>&& rhs)
    {
        _set = std::forward<std::set<CompoundElement, Comperator>>(rhs._set);
    }
            
    DDDeleteField(const DDDeleteField&) = delete;
    const DDDeleteField& operator=(const DDDeleteField&) = delete;
    
    void addIdx(IdxType idx)
    {
        auto ret = _set.insert(CompoundElement(idx));
        
        //adjust elemts diff count;
        if (ret.second==false)
        {
            ret.first->diff++;
        }
        else if (ret.second==true && ret.first != _set.begin())
        {
            auto lastElItr = ret.first;
            lastElItr--;
            ret.first->diff += lastElItr->diff;
        }
        
        //
        // check corner case: if the deletion point is above another deletion point.
        //
        auto tempItr = ret.first;
        
        ret.first++;
        
        if (ret.first != _set.end())
        {
            if (tempItr->idx == ret.first->idx - 1)
            {
                tempItr->diff += ret.first->diff;
                _set.erase(ret.first);
                tempItr++;
                ret.first = tempItr;
            }
        }
        //
        //
        
        adjustDiffs(ret.first);
    }
       
    //
    //iterator interface.
    typedef typename std::set<CompoundElement, Comperator>::iterator BoundItr;
    DDFieldIterator<IdxType, DDDeleteField<IdxType>, Dummy> fieldItr;
    //
            
    IdxType eval(IdxType idx)
    {
        if (_set.size() > 0)
        {
            auto itr = _set.upper_bound(CompoundElement(idx));
            
            if (itr != _set.begin())
            {
                itr--;
                idx += itr->diff;
            }
        }
        
        return idx;
    }
    
    void clear()
    {
        _set.clear();
    }

private:
    SetType _set;
    
    void adjustDiffs(typename SetType::iterator& itrIN)
    {
        for (typename SetType::iterator itr = itrIN; itr != _set.end(); itr++)
        {
            itr->diff++;
        }
    }
      
    //
    //iterator interface.
    friend class DDFieldIterator<IdxType, DDDeleteField<IdxType>, Dummy>;
            
    typename SetType::iterator beginItr()
    {
        return _set.begin();
    }
     
    IdxType eval(IdxType idx, typename SetType::iterator& itr)
    {
        
        if (itr != _set.end() && itr->idx <= idx) itr++;
        
        auto tempItr = itr;
        
        if (tempItr != _set.begin())
        {
            tempItr--;
            idx += tempItr->diff;
        }
    
        return idx;
    }
    //
    //
};

#endif
