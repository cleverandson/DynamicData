/*
 
    Copyright (c) 2013, Clever & Son
    All rights reserved.

    Redistribution and use in source and binary forms, with or without modification, are
    permitted provided that the following conditions are met:

    Redistributions of source code must retain the above copyright notice, this list of
    conditions and the following disclaimer.
    Redistributions in binary form must reproduce the above copyright notice, this list of
    conditions and the following disclaimer in the documentation and/or other materials
    provided with the distribution.
    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND
    CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
    INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
    MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
    CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
    SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
    NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
    CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
    STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
    ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
    ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/


#ifndef DynamicData_DDBaseVec_h
#define DynamicData_DDBaseVec_h

#include <set>
#include <list>
#include <vector>
#include <iostream>

/*
 * Requirements class BaseElementType
 * Default const  -- assing copy.
 * void adjust() const
 * IdxType base() const
 */

/*
 * Requirements class Element
 * Element(IdxType idx, const Element&& element, const BaseElementType& baseElement)
 * Default const -- assing copy.
 * Element(const Element&& other)
 * IdxType idxImp(const BaseElement<IdxType>& baseElement) const
 */

//TODO rename BaseElementType.
template<typename IdxType, class Element, class BaseElementType, size_t WindowWidth>
class DDBaseVec
{
private:
    
    class BaseElement;
    
    class BaseElementHandle
    {
    public:
        
        BaseElementHandle(IdxType idx) : _idx(idx)
        {}
    
        IdxType idx()
        {
            return _idx;
        }
        
    private:
        IdxType _idx;
        
        friend BaseElement;
        
        void adjust()
        {
            _idx++;
        }
    };
    
    class BaseElementHandles;
    
    class BaseHandlePtr
    {
    public:
      
        BaseHandlePtr() : _handlesPtr(0) {}
        
        BaseHandlePtr(IdxType idx, BaseElementHandles* handlesPtr) :
            _idx(idx),
            _handlesPtr(handlesPtr)
        {}
        
        BaseElementHandle* operator->()
        {
            //std::cout << "__aa " << _handlesPtr->size() << std::endl;
            
            assert(_handlesPtr);
            return &_handlesPtr->getHandle(_idx);
        }
        
        //TODO rename baseHandleIdx
        IdxType baseHandleIdx()
        {
            return _idx;
        }
                
    private:
        IdxType _idx;
        BaseElementHandles* _handlesPtr;
    };
    
    class BaseElementHandles
    {
    public:

        BaseHandlePtr createHandle(IdxType idx)
        {
            _handles.insert(_handles.end(), BaseElementHandle(idx));
            
            return BaseHandlePtr((IdxType)_handles.size()-1, this);
        }
        
        void clear()
        {
            _handles.clear();
        }
        
        //TODO remvove dbug
        IdxType size()
        {
            return (IdxType)_handles.size();
        }
        
    private:
        std::vector<BaseElementHandle> _handles;
        
        friend BaseHandlePtr;
        
        BaseElementHandle& getHandle(IdxType idx)
        {
            assert(_handles.size() > idx);
            return *(_handles.begin() + idx);
        }
        
    };
    
    class BaseElement : public BaseElementType
    {
    public:
        
        BaseElement(const BaseElement& other, IdxType leafElemCount, BaseHandlePtr baseHandlePtr) :
        BaseElementType(other),
        _leafElemCount(leafElemCount),
        _baseHandlePtr(baseHandlePtr)
        {}
        
        BaseElement(BaseHandlePtr baseHandlePtr) :
        BaseElementType(),
        _leafElemCount(0),
        _baseHandlePtr(baseHandlePtr)
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
        
        BaseHandlePtr baseHandlePtr()
        {
            return _baseHandlePtr;
        }
        
        IdxType baseContainerIdx()
        {
            return _baseHandlePtr->idx();
        }
        
        IdxType baseHandleIdx()
        {
            return _baseHandlePtr.baseHandleIdx();
        }
        
        //NEEDED!!
        void adjustHandle()
        {
            _baseHandlePtr->adjust();
        }
        
    private:
        mutable IdxType _leafElemCount;
        BaseHandlePtr _baseHandlePtr;
    };
    
    
    typedef std::vector<BaseElement> BaseContainer;
    
    typedef typename BaseContainer::iterator BasePtr;
    
    
    class BaseElementResolver
    {
    public:
        
        BaseElementResolver() {}
        
        BaseElementResolver(BaseHandlePtr baseHandlePtr, BaseContainer* baseVecPtr) :
            _baseHandlePtr(baseHandlePtr),
            _baseVecPtr(baseVecPtr)
        {}
        
        BasePtr basePtr() const
        {
            assert(_baseVecPtr->size() > _baseHandlePtr->idx());
            return _baseVecPtr->begin() + _baseHandlePtr->idx();
        }
        
        IdxType baseContainerIdx() const
        {
            return _baseHandlePtr->idx();
        }
        
        IdxType baseHandleIdx() const
        {
            return _baseHandlePtr.baseHandleIdx();
        }
        
        void newHandlePtr(const BaseHandlePtr& baseHandlePtr) const
        {
            _baseHandlePtr = baseHandlePtr;
        }
        
    private:
        mutable BaseHandlePtr _baseHandlePtr;
        BaseContainer* _baseVecPtr;
    };
    
    
    //TODO try to have the client deleted uses mehts.
    class LeafElement : public Element
    {
    public:
        
        LeafElement(IdxType idxIN, const Element&& element, BaseElementResolver&& resolver) :
            Element(idxIN, std::move(element), *resolver.basePtr()),
            _resolver(std::move(resolver)),
            _hasResolver(true),
            _fakeIdx(0)
        {}
        
        LeafElement(IdxType idx) :
            _hasResolver(false),
            _fakeIdx(idx)
        {}
        
        IdxType idx() const
        {
            return _hasResolver ? Element::idxImp(*_resolver.basePtr()) : _fakeIdx;
        }
        
        BasePtr basePtr() const
        {
            assert(_hasResolver);
            return _resolver.basePtr();
        }
        
        IdxType baseContainerIdx() const
        {
            return _resolver.baseContainerIdx();
        }
        
        IdxType baseHandleIdx() const
        {
            return _resolver.baseHandleIdx();
        }
        
        void newHandlePtr(const BaseHandlePtr& baseHandlePtr) const
        {
            if (_hasResolver) _resolver.newHandlePtr(baseHandlePtr);
        }
        
    private:
        mutable bool _hasResolver;
        IdxType _fakeIdx;
        
        BaseElementResolver _resolver;
    };
    
    class Comperator
    {
    public:
        bool operator() (const LeafElement& lhs, const LeafElement& rhs) const { return lhs.idx() < rhs.idx(); }
    };
    
    typedef std::set<LeafElement, Comperator> LeafContainer;
    typedef typename LeafContainer::iterator LeafPtr;
    
    
public:
    
    DDBaseVec() :
    _halfWindowWidth(WindowWidth / 2.0)
    {
        initBaseSet();
    }
    
    DDBaseVec(const DDBaseVec&) = delete;
    const DDBaseVec& operator=(const DDBaseVec&) = delete;
    
    void insert(LeafPtr insertPtr, IdxType idx, const Element& element) = delete;
    
    //assert that this idx is not present in this DDBaseVec!
    void insert(LeafPtr insertPtr, IdxType idx, const Element&& element)
    {
        //get the corresponding base element.
        BasePtr basePtr;
        if (_baseContainer.size() == 0) basePtr = _baseContainer.begin();
        else if (insertPtr == _leafSet.end())
        {
            basePtr = _baseContainer.end();
            basePtr--;
        }
        else basePtr = insertPtr->basePtr();
     
        bool hasNewBaseElement = false;
        
        //check if we have to insert a new BaseElement.
        if (basePtr->leafElemCount() >= WindowWidth)
        {
            //std::cout << "__new base " << std::endl;
            
            auto windowSearchItr = insertPtr;
            if (insertPtr == _leafSet.end()) windowSearchItr--;
            
            //get the beginning of the bucket.
            while (windowSearchItr != _leafSet.begin() && windowSearchItr->baseHandleIdx() == basePtr->baseHandleIdx())
            {
                windowSearchItr--;
            }
            if (windowSearchItr != _leafSet.begin()) windowSearchItr++;
            
            
            //advance to the base insertion idx.
            advance(windowSearchItr, _halfWindowWidth);
            
            //adjust the leaf elements of the base ptr.
            IdxType nextLeafElemCount = basePtr->leafElemCount() - _halfWindowWidth;
            basePtr->setLeafElemCount(_halfWindowWidth);
            
            IdxType baseInsertIdx = windowSearchItr->idx();
            
            //optimization indicates wether the new base handles containerIdx should be adjusted now or when
            //adjustNodesImp is invocated next.
            bool shouldAdjustBaseHandle = (idx >= baseInsertIdx);
            
            //get the new base node.
            BaseElement insBaseElement(newBaseElement((*basePtr), nextLeafElemCount, shouldAdjustBaseHandle));
             
            
            //before adding the base node to the container, add it to the leaf elements. otherwise the baseContainerIdx would not be correct.
            while((windowSearchItr != _leafSet.end()) && (windowSearchItr->baseHandleIdx() == basePtr->baseHandleIdx()))
            {
                windowSearchItr->newHandlePtr(insBaseElement.baseHandlePtr());
                windowSearchItr++;
            }
            
            //insert the new base node and get an iterator.
            auto newBasePtr = basePtr;
            newBasePtr++;
            basePtr = _baseContainer.insert(newBasePtr, insBaseElement);
            basePtr--;
            
            //check if we have to adjust the base ptr.
            if (idx >= baseInsertIdx)
            {
                basePtr++;
            }
            
            hasNewBaseElement = true;
        }
        
    
        //get the next base ptr.
        auto nextBaseSetPtr = basePtr;
        nextBaseSetPtr++;
        
        
        //adjust the nodes
        adjustNodesImp(insertPtr, basePtr, nextBaseSetPtr, hasNewBaseElement);
        
        //std::cout << "__dd " << basePtr->baseHandlePtr()->idx() << std::endl;
        //insert the leaf element
        _leafSet.insert(insertPtr, LeafElement(idx, std::move(element), BaseElementResolver(basePtr->baseHandlePtr(), &_baseContainer)));
        
        
        //adjust the bucket size.
        basePtr->incrLeafElemCount();
    }
    
    void adjust(LeafPtr leafPtr)
    {
        assert(leafPtr != _leafSet.end());
        
        auto leafsBasePtr = _baseContainer.begin() + leafPtr->baseContainerIdx();
        auto nextBasePtr = leafsBasePtr;
        nextBasePtr++;
        
        //auto basePtr = leafPtr->baseContainerIdx();
        //basePtr++;
        
        adjustNodesImp(leafPtr, leafsBasePtr, nextBasePtr, false);
    }
    
    typename LeafContainer::iterator upperBound(IdxType idx)
    {
        //TODO possible optimization on the creation of LeafElement. make it static?
        return _leafSet.upper_bound(LeafElement(idx));
    }
    
    typename LeafContainer::iterator equalRange(IdxType idx)
    {
        return _leafSet.equal_range(LeafElement(idx)).second;
    }
    
    typename LeafContainer::iterator begin()
    {
        return _leafSet.begin();
    }
    
    typename LeafContainer::iterator end()
    {
        return _leafSet.end();
    }
    
    size_t size()
    {
        return _leafSet.size();
    }
    
    void clear()
    {
        _leafSet.clear();
        
        initBaseSet();
    }
    
    //
    //DEBUG
    typename BaseContainer::iterator baseBegin()
    {
        return _baseContainer.begin();
    }
    
    typename BaseContainer::iterator baseEnd()
    {
        return _baseContainer.end();
    }
    //
    //
    
    typedef typename LeafContainer::iterator iterator;
    
private:
    
    LeafContainer _leafSet;
    
    //TODO rename baseSet.
    BaseContainer _baseContainer;
    BaseElementHandles _baseElementHandles;
    
    IdxType _halfWindowWidth;
    
    void initBaseSet()
    {
        _baseElementHandles.clear();
        _baseContainer.clear();
        _baseContainer.insert(_baseContainer.begin(),BaseElement(_baseElementHandles.createHandle(0)));
    }
    
    BaseElement newBaseElement(BaseElement& prevBaseElement, IdxType leafElemCount, bool adjustBaseHandle)
    {
        if (adjustBaseHandle)
        {
            return BaseElement(prevBaseElement, leafElemCount, _baseElementHandles.createHandle(prevBaseElement.baseContainerIdx()+1));
        }
        else
        {
            return BaseElement(prevBaseElement, leafElemCount, _baseElementHandles.createHandle(prevBaseElement.baseContainerIdx()));
        }
    }
    
    void adjustNodesImp(LeafPtr leafPtr, BasePtr leafPtrsBasePtr, BasePtr basePtr, bool adjustHandle)
    {
        //adjust the base elements.
        auto currBasePtr = basePtr;
        while (currBasePtr != _baseContainer.end())
        {
            currBasePtr->adjust();
            if (adjustHandle)
            {
                currBasePtr->adjustHandle();
            }
            
            currBasePtr++;
        }
        
        //adjust the leaf elements.
        auto currPtr = leafPtr;
        while(currPtr != _leafSet.end() && currPtr->baseHandleIdx() == leafPtrsBasePtr->baseHandleIdx())
        {
            currPtr->adjust();
            currPtr++;
        }
    }
};

#endif
