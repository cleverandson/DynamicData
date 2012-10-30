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

template<typename IdxType>
class DDDeleteField
{
private:
    
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
    
public:
    
    DDDeleteField() {}
    
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
        
        ret.first++;
        
        adjustDiffs(ret.first);
    }
    
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
    
    /*
     void debugPrint()
     {
     for (typename SetType::iterator itr = _set.begin(); itr != _set.end(); itr++)
     {
     std::cout << "__ " << itr->diff << std::endl;
     }
     }
     */
    
private:
    
    typedef std::set<CompoundElement, Comperator> SetType;
    
    SetType _set;
    
    void adjustDiffs(typename SetType::iterator& itrIN)
    {
        for (typename SetType::iterator itr = itrIN; itr != _set.end(); itr++)
        {
            itr->diff++;
        }
    }

};

#endif
