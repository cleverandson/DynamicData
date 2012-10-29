//
//  DDMonotonic.h
//  DynamicData
//
//  Created by mich2 on 10/25/12.
//  Copyright (c) 2012 -. All rights reserved.
// 

#ifndef DynamicData_DDMonotonic_h
#define DynamicData_DDMonotonic_h

#include <set>

template<typename IdxType, class CachedElement>
class DDMonotonic
{
private:

    class Dummy {};
    
    template<typename IdxTypeIternal, typename CachedElementInternal, typename Dummy>
    class CompoundElement
    {
    public:
        
        CompoundElement(IdxTypeIternal idxIN) :
            idx(idxIN),
            diff(0)
        { }
        
        CompoundElement(IdxTypeIternal idxIN, CachedElementInternal cachedElementIN, int adjustValue) :
            cachedElements(1, cachedElementIN),
            idx(idxIN),
            diff(adjustValue)
        { }
        
        IdxTypeIternal idx;
        mutable IdxType diff;
        mutable std::vector<CachedElementInternal> cachedElements;
    };
    
    template<typename Dummy>
    class CompoundElement<IdxType, IdxType, Dummy>
    {
    public:
        
        CompoundElement(IdxType idxIN) :
            idx(idxIN),
            diff(0)
        { }
        
        CompoundElement(IdxType idxIN, int adjustValue) :
            idx(idxIN),
            diff(adjustValue)
        { }
        
        IdxType idx;
        mutable IdxType diff;
    };
    
    template<typename CompoundElement>
    class Comperator
    {
    public:
        
        bool operator() (const CompoundElement& lhs, const CompoundElement& rhs) const
        {
            return lhs.idx < rhs.idx;
        }
    };
    
public:
    
    DDMonotonic(int adjustValue) :
        _adjustValue(adjustValue)
    {
        
    }
    
    DDMonotonic(const DDMonotonic&) = delete;
    const DDMonotonic& operator=(const DDMonotonic&) = delete;
            
    void addIdx(IdxType idx)
    {
        auto ret = _set.insert(CompoundElement<IdxType, IdxType, Dummy>(idx, _adjustValue));
    
        //adjust elemts diff count;
        if (ret.second==false)
        {
            ret.first->diff += _adjustValue;
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
    
    void addIdx(IdxType idx, const CachedElement& cachedElement)
    {
        auto ret = _set.insert(CompoundElement<IdxType, CachedElement, Dummy>(idx, cachedElement, _adjustValue));
    
        if (ret.second==false)
        {
            ret.first->cachedElements.push_back(cachedElement);
            ret.first->diff += _adjustValue;
        }
        else if (ret.second==true && ret.first != _set.begin())
        {
            auto lastElItr = ret.first;
            lastElItr--;
            ret.first->diff = lastElItr->diff + _adjustValue;
        }
        
        ret.first++;
        
        adjustDiffs(ret.first);
    }
            
    IdxType eval(IdxType idx)
    {
        if (_set.size() > 0)
        {
            auto itr = _set.upper_bound(CompoundElementType(idx));
            
            if (itr != _set.begin())
            {
                itr--;
                idx += itr->diff;
            }
        }
        
        return idx;
    }
    
    IdxType eval(IdxType idx, bool& hasCacheElement, CachedElement& cachedElement)
    {
        hasCacheElement = false;
        
        if (_set.size() > 0)
        {
            auto itr = _set.upper_bound(CompoundElementType(idx));
            
            if (itr != _set.end())
            {
                auto& ce = (*itr);
                IdxType idxDiff = ce.idx - idx - 1;
                
                if (idxDiff < ce.cachedElements.size())
                {
                    hasCacheElement = true;
                    cachedElement = ce.cachedElements[idxDiff];
                }
            }
            
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
    
    typedef CompoundElement<IdxType, CachedElement, Dummy> CompoundElementType;
            
    typedef Comperator<CompoundElementType> ComperatorType;
            
    typedef std::set<CompoundElementType, ComperatorType> SetType;
            
    SetType _set;
    int _adjustValue;
    
    void adjustDiffs(typename SetType::iterator& itrIN)
    {
        for (typename SetType::iterator itr = itrIN; itr != _set.end(); itr++)
        {
            itr->diff += _adjustValue;
        }
    }
};

#endif
