//
//  DDBaseSet.h
//  DynamicData
//
//  Created by mich2 on 11/21/12.
//  Copyright (c) 2012 -. All rights reserved.
//

#ifndef DynamicData_DDBaseSet_h
#define DynamicData_DDBaseSet_h

#include <set>
#include <list>
#include <iostream>

/*
 * Requirements class BaseElementType
 * Default const assing const.
 * void adjust() const
 * IdxType base() const
*/

/*
 * Requirements class Element
 * Element(IdxType idx, const Element&& element, const BaseElementType& baseElement)
 * Default const assing const.
 * Element(const Element&& other)
 * IdxType idxImp(const BaseElement<IdxType>& baseElement) const
*/

//TODO rename BaseElementType.
template<typename IdxType, class Element, class BaseElementType, size_t WindowWidth>
class DDBaseSet
{
private:
       
    class BaseElement : public BaseElementType
    {
    public:
        BaseElement(const BaseElement& other, IdxType leafElemCount) :
            BaseElementType(other),
            _leafElemCount(leafElemCount)
        {}
        
        BaseElement() :
            BaseElementType(),
            _leafElemCount(0)
        {}
        
        IdxType leafElemCount() const
        {
            return _leafElemCount;
        }
        
        void setLeafElemCount(IdxType leafElemCount) const
        {
            _leafElemCount = leafElemCount;
        }
        void incrLeafElemCount() const
        {
            _leafElemCount++;
        }
        
    private:
        mutable IdxType _leafElemCount;
    };
    
    
    //TODO rename
    typedef std::list<BaseElement> BaseSetType;
    typedef typename BaseSetType::iterator BaseSetPtr;

    //TODO try to have the client deleted uses mehts.
    class LeafElement : public Element
    {
    public:
        
        LeafElement(IdxType idxIN, const Element&& element, BaseSetPtr baseSetPtrIN) :
            Element(idxIN, std::move(element), *baseSetPtrIN),
            baseSetPtr(baseSetPtrIN),
            _hasBaseSetPtr(true),
            _fakeIdx(0)
        {}
        
        LeafElement(IdxType idx) :
            _hasBaseSetPtr(false),
            _fakeIdx(idx)
        {}
        
        //TODO rename
        IdxType idx() const
        {
            return _hasBaseSetPtr ? Element::idxImp(*baseSetPtr) : _fakeIdx;
        }
        
        //TODO make idx private
        //mutable IdxType idx;
        mutable BaseSetPtr baseSetPtr;
        
    private:
        mutable bool _hasBaseSetPtr;
        IdxType _fakeIdx;
    };

    class Comperator
    {
    public:
        bool operator() (const LeafElement& lhs, const LeafElement& rhs) const { return lhs.idx() < rhs.idx(); }
    };
    
    typedef std::set<LeafElement, Comperator> LeafSetType;
    typedef typename LeafSetType::iterator LeafSetPtr;
    
    
public:
    
    DDBaseSet() :
        _halfWindowWidth(WindowWidth / 2.0)
    {
        initBaseSet();
    }
            
    void insert(LeafSetPtr insertPtr, IdxType idx, const Element& element) = delete;
    
    //assert that this idx is not present in this DDBaseSet!
    void insert(LeafSetPtr insertPtr, IdxType idx, const Element&& element)
    {
        BaseSetPtr basePtr;
        
        //get the base ptr.
        if (_leafSet.size() == 0) basePtr = _baseSet.begin();
        else
        {
            if (insertPtr == _leafSet.end())
            {
                auto tempItr = insertPtr;
                tempItr--;
                
                basePtr = tempItr->baseSetPtr;
            }
            else basePtr = insertPtr->baseSetPtr;
        }
        
        //check if we have to insert a new BaseElement.
        if (basePtr->leafElemCount() > WindowWidth)
        {
            auto windowSearchItr = insertPtr;
            if (insertPtr == _leafSet.end()) windowSearchItr--;
            
            //get the beginning of the bucket.
            while (windowSearchItr != _leafSet.begin() && windowSearchItr->baseSetPtr == basePtr)
            {
                windowSearchItr--;
            }
            if (windowSearchItr != _leafSet.begin()) windowSearchItr++;
            
            //advance to the base insertion idx.
            advance(windowSearchItr, _halfWindowWidth);
            
            //adjust the leaf elements of the base ptr.
            basePtr->setLeafElemCount(_halfWindowWidth);
            IdxType nextLeafElemCount = basePtr->leafElemCount() - _halfWindowWidth;
            
            IdxType baseInsertIdx = windowSearchItr->idx();
            
            //consruct the new base node.
            BaseElement baseElement((*basePtr),nextLeafElemCount);
            
            //insert the new base node and get an iterator.
            auto newBasePtr = basePtr;
            newBasePtr++;
            _baseSet.insert(newBasePtr, baseElement);
            newBasePtr--;
            
            //add the new base node to the corresponding leaf elements.
            while(windowSearchItr != _leafSet.end() && windowSearchItr->baseSetPtr == basePtr)
            {
                windowSearchItr->baseSetPtr = newBasePtr;
                windowSearchItr++;
            }
            
            //check if we have to adjust the base ptr.
            if (idx >= baseInsertIdx) basePtr = newBasePtr;
        }
        
        //get the next base ptr.
        auto nextBaseSetPtr = basePtr;
        nextBaseSetPtr++;
        
        
        //adjust the nodes
        adjustNodesImp(insertPtr, basePtr, nextBaseSetPtr);
        
        
        //insert the leaf element
        _leafSet.insert(insertPtr, LeafElement(idx, std::move(element), basePtr));
        
        //adjust the bucket size.
        basePtr->incrLeafElemCount();
    }
    
    void adjust(LeafSetPtr leafPtr)
    {
        assert(leafPtr != _leafSet.end());
        
        auto basePtr = leafPtr->baseSetPtr;
        basePtr++;
        
        adjustNodesImp(leafPtr, leafPtr->baseSetPtr, basePtr);
    }
    
    typename LeafSetType::iterator upperBound(IdxType idx)
    {
        //TODO possible optimization on the creation of LeafElement. make it static?
        return _leafSet.upper_bound(LeafElement(idx));
    }
          
    typename LeafSetType::iterator equalRange(IdxType idx)
    {
        return _leafSet.equal_range(LeafElement(idx)).second;
    }

    typename LeafSetType::iterator begin()
    {
        return _leafSet.begin();
    }
            
    typename LeafSetType::iterator end()
    {
        return _leafSet.end();
    }
    
    size_t size()
    {
        return _leafSet.size();
    }
    
    void clear()
    {
        initBaseSet();
        _leafSet.clear();
    }
    
    //
    //DEBUG
    typename BaseSetType::iterator baseBegin()
    {
        return _baseSet.begin();
    }
    
    typename BaseSetType::iterator baseEnd()
    {
        return _baseSet.end();
    }
    //
    //
    
    typedef typename LeafSetType::iterator iterator;
    
private:
    
    LeafSetType _leafSet;
    BaseSetType _baseSet;
    
    IdxType _halfWindowWidth;
    
    void initBaseSet()
    {
        _baseSet.clear();
        _baseSet.insert(_baseSet.begin(),BaseElement());
    }
    
    void adjustNodesImp(LeafSetPtr leafPtr, BaseSetPtr leafPtrsBasePtr, BaseSetPtr basePtr)
    {
        //adjust the base elements.
        auto currBasePtr = basePtr;
        while (currBasePtr != _baseSet.end())
        {
            currBasePtr->adjust();
            currBasePtr++;
        }
        
        //adjust the leaf elements.
        auto currPtr = leafPtr;
        while(currPtr->baseSetPtr == leafPtrsBasePtr)
        {
            currPtr->adjust();
            currPtr++;
        }
    }
};
            
#endif
