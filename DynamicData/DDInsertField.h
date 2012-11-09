//
//  DDInsertField.h
//  DynamicData
//
//  Created by mich2 on 10/30/12.
//  Copyright (c) 2012 -. All rights reserved.
//

#ifndef DynamicData_DDInsertField_h
#define DynamicData_DDInsertField_h

#include <vector>
#include "DDFieldIterator.h"

template<typename IdxType, class CachedElement>
class DDInsertField
{
private:
    
    class Element
    {
    public:
        Element(IdxType idxIN) :
            idx(idxIN),
            diff(1)
        {}
        
        Element(IdxType idxIN, const CachedElement& cachedElement) :
            idx(idxIN),
            diff(1),
            cachedElements(1, cachedElement)
        {}

        Element(IdxType idxIN, IdxType diffIN, const CachedElement& cachedElement) :
        idx(idxIN),
        diff(diffIN),
        cachedElements(1, cachedElement)
        {}
        
        IdxType idx;
        IdxType diff;
        std::vector<CachedElement> cachedElements;
    };
    
    class Comperator
    {
    public:
        
        bool operator() (const Element& lhs, const Element& rhs) const
        {
            return lhs.idx < rhs.idx;
        }
    };
            
public:
    
    DDInsertField() :
        fieldItr(*this)
    {}
            
    DDInsertField(DDInsertField&& other) :
        fieldItr(*this),
        _vec(std::forward<std::vector<Element>>(other._vec))
    {}
            
    void operator=(DDInsertField&& rhs)
    {
        _vec = std::forward<std::vector<Element>>(rhs._vec);
    }

    DDInsertField(const DDInsertField&) = delete;
    const DDInsertField& operator=(const DDInsertField&) = delete;
    
    void addIdx(IdxType idx, const CachedElement& cachedElement)
    {
        auto biggerThanItr = std::upper_bound(_vec.begin(), _vec.end(), Element(idx), Comperator());
    
        if (biggerThanItr == _vec.begin())
        {
            biggerThanItr = _vec.insert(_vec.begin(), Element(idx, cachedElement));
            biggerThanItr++;
        }
        else
        {
            auto smallerOrEqual = biggerThanItr;
            smallerOrEqual--;
        
            if (smallerOrEqual->idx != idx)
            {
                biggerThanItr = _vec.insert(biggerThanItr, Element(idx, smallerOrEqual->diff + 1, cachedElement));
                biggerThanItr++;
            }
            else
            {
                smallerOrEqual->idx++;
                smallerOrEqual->diff++;
                smallerOrEqual->cachedElements.push_back(cachedElement);
            }
        }

        adjustIdxs(biggerThanItr);
    }

    IdxType eval(IdxType idx, bool& hasCacheElement, CachedElement& cachedElement)
    {
        auto biggerThanItr = std::equal_range(_vec.begin(), _vec.end(), Element(idx), Comperator()).second;

        return evalImpl(idx, biggerThanItr, hasCacheElement, cachedElement);
    }
            
    void debugPrint()
    {
        for (typename std::vector<Element>::iterator itr = _vec.begin(); itr != _vec.end(); itr++)
        {
            std::cout << "_i_ " << itr->idx << "_d_ " << itr->diff << std::endl;
        }
    }

    //
    //iterator interface.
    typedef typename std::vector<Element>::iterator BoundItr;
    DDFieldIterator<IdxType, DDInsertField<IdxType, CachedElement>, CachedElement> fieldItr;
    //
    
    void clear()
    {
        _vec.clear();
    }

private:
    std::vector<Element> _vec;

    void adjustIdxs(typename std::vector<Element>::iterator& itr)
    {
        while (itr != _vec.end())
        {
            itr->idx++;
            itr->diff++;
            itr++;
        }
    }
            
    //
    //iterator interface.
    friend class DDFieldIterator<IdxType, DDInsertField<IdxType, CachedElement>, CachedElement>;
            
    typename std::vector<Element>::iterator beginItr()
    {
        return _vec.begin();
    }

    IdxType eval(IdxType idx, typename std::vector<Element>::iterator& biggerThanItr, bool& hasCacheElement, CachedElement& cachedElement)
    {
        while (biggerThanItr != _vec.end() && biggerThanItr->idx <= idx)
        {
            biggerThanItr++;
        }
        
        return evalImpl(idx, biggerThanItr, hasCacheElement, cachedElement);
    }
    //
    //
    
    IdxType evalImpl(IdxType idx, typename std::vector<Element>::iterator& biggerThanItr, bool& hasCacheElement, CachedElement& cachedElement)
    {
        hasCacheElement = false;
        
        //check for cached values.
        if (biggerThanItr != _vec.end())
        {
            IdxType idxDiff = biggerThanItr->idx - idx - 1;
            
            size_t numbOfElements = biggerThanItr->cachedElements.size();
            
            if (idxDiff < numbOfElements)
            {
                cachedElement = biggerThanItr->cachedElements[idxDiff];
                hasCacheElement = true;
            }
        }
        
        if (biggerThanItr != _vec.begin())
        {
            auto smallerOrEqual = biggerThanItr;
            smallerOrEqual--;
            
            idx -= smallerOrEqual->diff;
        }
        
        return idx;
    }
};

#endif
