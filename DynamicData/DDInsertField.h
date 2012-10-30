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
    
    DDInsertField() {}
            
    DDInsertField(const DDInsertField&) = delete;
    const DDInsertField& operator=(const DDInsertField&) = delete;
    
    void addIdx(IdxType idx, const CachedElement& cachedElement)
    {
        auto biggerThanItr = std::equal_range(_vec.begin(), _vec.end(), Element(idx), Comperator()).second;
    
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
                biggerThanItr = _vec.insert(biggerThanItr, Element(idx, cachedElement));
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
        hasCacheElement = false;
        
        auto biggerThanItr = std::equal_range(_vec.begin(), _vec.end(), Element(idx), Comperator()).second;
        
        //check for cached values.
        if (biggerThanItr != _vec.end())
        {
            IdxType idxDiff = biggerThanItr->idx - idx - 1;
            
            size_t numbOfElements = biggerThanItr->cachedElements.size();
            
            if (idxDiff < numbOfElements)
            {
                cachedElement = biggerThanItr->cachedElements[numbOfElements - idxDiff - 1];
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
            
    void debugPrint()
    {
        for (typename std::vector<Element>::iterator itr = _vec.begin(); itr != _vec.end(); itr++)
        {
            std::cout << "__ " << itr->diff << std::endl;
        }
    }

private:
    std::vector<Element> _vec;

    void adjustIdxs(typename std::vector<Element>::iterator& itr)
    {
        while (itr != _vec.end())
        {
            itr->idx++;
            itr++;
        }
    }
};

#endif
