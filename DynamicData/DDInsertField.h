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
#include "DDBaseSet.h"

template<typename IdxType, class CachedElement>
class DDInsertField
{
private:
    
    
    //
    //
    class BaseElement
    {
    public:
        
        BaseElement() : _base(0) {}
        
        void adjust() const
        {
            _base++;
        }
        
        IdxType base() const
        {
            return _base;
        }
        
    private:
        mutable IdxType _base;
    };
    
    class Element2
    {
    public:
        
        Element2() = default;
        
        Element2(IdxType idxIN, IdxType diffIN, const CachedElement& cachedElement) :
            _relIdx(idxIN),
            _relDiff(diffIN),
            cachedElements(1, cachedElement)
        {}
        
        Element2(IdxType idx, const Element2&& element, const BaseElement& baseElement) :
            cachedElements(std::move(element.cachedElements))
        {
            _relIdx = idx - baseElement.base();
            _relDiff = element._relDiff - baseElement.base();
        }
        
        void adjust() const
        {
            _relIdx++;
            _relDiff++;
        }
        
        IdxType idxImp(const BaseElement& baseElement) const
        {
            return _relIdx + baseElement.base();
        }
        
        IdxType diffImp(const BaseElement& baseElement) const
        {
            return _relDiff + baseElement.base();
        }

        mutable std::deque<CachedElement> cachedElements;
        
    private:
        mutable IdxType _relIdx;
        mutable IdxType _relDiff;
    };
    
    template<class DDBaseSetLeafElement>
    static IdxType getDiff(DDBaseSetLeafElement& leafElement)
    {
        return leafElement.diffImp(*leafElement.baseSetPtr);
    }
    
    typedef DDBaseSet<IdxType, Element2, BaseElement, 80> InsertContainer;

public:
    
    DDInsertField() :
        fieldItr(*this)
    {}
            
    DDInsertField(DDInsertField&& other) :
        fieldItr(*this),
        _ddBaseSet(std::forward<InsertContainer>(other._ddBaseSet))
    {}
    
    void operator=(DDInsertField&& rhs)
    {
        _ddBaseSet = std::forward<InsertContainer>(rhs._ddBaseSet);
    }
    
    DDInsertField(const DDInsertField&) = delete;
    const DDInsertField& operator=(const DDInsertField&) = delete;
    
    
            
            
    void addIdx(IdxType idx, const CachedElement& cachedElement)
    {
        auto biggerThanItr = _ddBaseSet.upperBound(idx);
     
        //
        // shadow case.
        //
        if (biggerThanItr != _ddBaseSet.end() && idx > (biggerThanItr->idx() - biggerThanItr->cachedElements.size()))
        {
            auto itr = biggerThanItr->cachedElements.begin();
            
            itr += biggerThanItr->idx() - idx + 1;
            
            biggerThanItr->cachedElements.insert(itr, cachedElement);
            
            _ddBaseSet.adjust(biggerThanItr);
        }
        else
        {
            bool caseMached = false;
            IdxType lastDiff = 0;
            
            if (biggerThanItr != _ddBaseSet.begin())
            {
                auto smallerOrEqual = biggerThanItr;
                smallerOrEqual--;
                
                lastDiff = getDiff(*smallerOrEqual);
                
                //
                //hit case.
                //
                if (idx == smallerOrEqual->idx())
                {
                    _ddBaseSet.adjust(smallerOrEqual);
                    
                    auto itr = smallerOrEqual->cachedElements.begin();
                    itr++;
                    
                    smallerOrEqual->cachedElements.insert(itr, cachedElement);
                    
                    caseMached = true;
                }
                //
                //+1 case.
                //
                else if (idx == smallerOrEqual->idx() + 1)
                {
                    _ddBaseSet.adjust(smallerOrEqual);
                    
                    smallerOrEqual->cachedElements.insert(smallerOrEqual->cachedElements.begin(), cachedElement);
                    
                    caseMached = true;
                }
            }
            
            //
            //insert case.
            //
            if (!caseMached)
            {
                _ddBaseSet.insert(biggerThanItr, idx, Element2(idx, lastDiff + 1, cachedElement));
            }
        }
    }
            
    IdxType eval(IdxType idx, bool& hasCacheElement, CachedElement& cachedElement)
    {
        auto biggerThanItr = _ddBaseSet.equalRange(idx);
        
        return evalImpl(idx, biggerThanItr, hasCacheElement, cachedElement);
    }
       
    /*
    void debugPrint()
    {
        for (typename std::vector<Element>::iterator itr = _insertContainer.begin(); itr != _insertContainer.end(); itr++)
        {
            std::cout << "_i_ " << itr->idx << "_d_ " << itr->diff << std::endl;
        }
    }
    */
            
    //
    //iterator interface.
    typedef typename InsertContainer::iterator BoundItr;
    DDFieldIterator<IdxType, DDInsertField<IdxType, CachedElement>, CachedElement> fieldItr;
    //
    
    void clear()
    {
        _ddBaseSet.clear();
    }

private:
    InsertContainer _ddBaseSet;
    
    //
    //iterator interface.
    friend class DDFieldIterator<IdxType, DDInsertField<IdxType, CachedElement>, CachedElement>;
            
    typename InsertContainer::iterator beginItr()
    {
        return _ddBaseSet.begin();
    }

    IdxType eval(IdxType idx, typename InsertContainer::iterator& biggerThanItr, bool& hasCacheElement, CachedElement& cachedElement)
    {
        while (biggerThanItr != _ddBaseSet.end() && biggerThanItr->idx() <= idx)
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
        
        if (biggerThanItr != _ddBaseSet.end())
        {
            IdxType idxDiff = biggerThanItr->idx() - idx - 1;
            
            
            size_t numbOfElements = biggerThanItr->cachedElements.size();
            
            if (idxDiff < numbOfElements)
            {
                cachedElement = biggerThanItr->cachedElements[idxDiff];
                hasCacheElement = true;
            }
        }
        
        if (biggerThanItr != _ddBaseSet.begin())
        {
            auto smallerOrEqual = biggerThanItr;
            smallerOrEqual--;
            
            idx -= getDiff(*smallerOrEqual);
        }
        
        return idx;
    }
};

#endif
