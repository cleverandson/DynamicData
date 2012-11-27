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
#include <deque>
#include <set>

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
        
        mutable IdxType idx;
        mutable IdxType diff;
        mutable std::deque<CachedElement> cachedElements;
    };
    
    class Comperator
    {
    public:
        
        bool operator() (const Element& lhs, const Element& rhs) const
        {
            return lhs.idx < rhs.idx;
        }
    };
            
    //typedef std::vector<Element> InsertContainer;
    typedef std::set<Element, Comperator> InsertContainer;
    //typedef std::deque<Element> InsertContainer;
            
public:
    
    DDInsertField() :
        fieldItr(*this)
    {}
            
    DDInsertField(DDInsertField&& other) :
        fieldItr(*this),
        _insertContainer(std::forward<InsertContainer>(other._insertContainer))
    {}
            
    void operator=(DDInsertField&& rhs)
    {
        _insertContainer = std::forward<InsertContainer>(rhs._insertContainer);
    }

    DDInsertField(const DDInsertField&) = delete;
    const DDInsertField& operator=(const DDInsertField&) = delete;
    
    
            
            
    void addIdx(IdxType idx, const CachedElement& cachedElement)
    {
        auto biggerThanItr = _insertContainer.upper_bound(Element(idx));
        //auto biggerThanItr = std::upper_bound(_insertContainer.begin(), _insertContainer.end(), Element(idx), Comperator());
        
        //
        //shadow case.
        //
        if (biggerThanItr != _insertContainer.end() && idx > (biggerThanItr->idx - biggerThanItr->cachedElements.size()))
        {
            //std::cout << "shadow case." << std::endl;
            
            auto itr = biggerThanItr->cachedElements.begin();
            itr += biggerThanItr->idx - idx + 1;
            
            //std::cout << "DD__ " << biggerThanItr->idx << " _aa_ " << idx << " __ " << biggerThanItr->idx - idx << " _size_ " << biggerThanItr->cachedElements.size() << std::endl;
            
            biggerThanItr->cachedElements.insert(itr, cachedElement);
            
            biggerThanItr->idx++;
            biggerThanItr->diff++;
            
            biggerThanItr++;
        }
        else
        {
            bool caseMached = false;
            IdxType lastDiff = 0;
            
            if (biggerThanItr != _insertContainer.begin())
            {
                auto smallerOrEqual = biggerThanItr;
                smallerOrEqual--;
                lastDiff = smallerOrEqual->diff;
                
                //
                //hit case.
                //
                if (idx == smallerOrEqual->idx)
                {
                    //std::cout << "hit case." << std::endl;
                    
                    smallerOrEqual->idx++;
                    smallerOrEqual->diff++;
                    
                    auto itr = smallerOrEqual->cachedElements.begin();
                    itr++;
                    
                    smallerOrEqual->cachedElements.insert(itr, cachedElement);
                
                    caseMached = true;
                }
                //
                //+1 case.
                //
                else if (idx == smallerOrEqual->idx + 1)
                {
                    //std::cout << "+1 case." << std::endl;
                    
                    smallerOrEqual->idx++;
                    smallerOrEqual->diff++;
                    
                    smallerOrEqual->cachedElements.insert(smallerOrEqual->cachedElements.begin(), cachedElement);
                
                    caseMached = true;
                }
            }
            
            //
            //insert case.
            //
            if (!caseMached)
            {
                //std::cout << "insert case." << std::endl;
                
                biggerThanItr = _insertContainer.insert(biggerThanItr, Element(idx, lastDiff + 1, cachedElement));
                biggerThanItr++;
            }
        }

        adjustIdxs(biggerThanItr);
    }
            
    IdxType eval(IdxType idx, bool& hasCacheElement, CachedElement& cachedElement)
    {
        auto biggerThanItr = _insertContainer.equal_range(Element(idx)).second;
        //auto biggerThanItr = std::equal_range(_insertContainer.begin(), _insertContainer.end(), Element(idx), Comperator()).second;

        return evalImpl(idx, biggerThanItr, hasCacheElement, cachedElement);
    }
            
    void debugPrint()
    {
        for (typename std::vector<Element>::iterator itr = _insertContainer.begin(); itr != _insertContainer.end(); itr++)
        {
            std::cout << "_i_ " << itr->idx << "_d_ " << itr->diff << std::endl;
        }
    }

    //
    //iterator interface.
    typedef typename InsertContainer::iterator BoundItr;
    DDFieldIterator<IdxType, DDInsertField<IdxType, CachedElement>, CachedElement> fieldItr;
    //
    
    void clear()
    {
        _insertContainer.clear();
    }

private:
    InsertContainer _insertContainer;

    void adjustIdxs(typename InsertContainer::iterator& itr)
    {
        while (itr != _insertContainer.end())
        {
            itr->idx++;
            itr->diff++;
            itr++;
        }
    }
            
    //
    //iterator interface.
    friend class DDFieldIterator<IdxType, DDInsertField<IdxType, CachedElement>, CachedElement>;
            
    typename InsertContainer::iterator beginItr()
    {
        return _insertContainer.begin();
    }

    IdxType eval(IdxType idx, typename InsertContainer::iterator& biggerThanItr, bool& hasCacheElement, CachedElement& cachedElement)
    {
        while (biggerThanItr != _insertContainer.end() && biggerThanItr->idx <= idx)
        {
            biggerThanItr++;
        }
        
        return evalImpl(idx, biggerThanItr, hasCacheElement, cachedElement);
    }
    //
    //
    
    IdxType evalImpl(IdxType idx, typename InsertContainer::iterator& biggerThanItr, bool& hasCacheElement, CachedElement& cachedElement)
    {
        hasCacheElement = false;
        
        //check for cached values.
        if (biggerThanItr != _insertContainer.end())
        {
            IdxType idxDiff = biggerThanItr->idx - idx - 1;
            
            size_t numbOfElements = biggerThanItr->cachedElements.size();
            
            if (idxDiff < numbOfElements)
            {
                cachedElement = biggerThanItr->cachedElements[idxDiff];
                hasCacheElement = true;
            }
        }
        
        if (biggerThanItr != _insertContainer.begin())
        {
            auto smallerOrEqual = biggerThanItr;
            smallerOrEqual--;
            
            idx -= smallerOrEqual->diff;
        }
        
        return idx;
    }
};

#endif
