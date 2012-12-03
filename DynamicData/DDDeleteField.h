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
#include "DDBaseSet.h"

template<typename IdxType>
class DDDeleteField
{
private:
    
    class Dummy{};
    
    //
    //
    class BaseElement
    {
    public:
        
        BaseElement() : _baseIdx(0), _baseDiff(0) {}
        
        void adjust() const
        {
            _baseIdx++;
            _baseDiff++;
        }
        
        void adjustIdx() const
        {
            _baseIdx++;
        }
        
        IdxType baseIdx() const
        {
            return _baseIdx;
        }
        
        IdxType baseDiff() const
        {
            return _baseDiff;
        }
        
    private:
        mutable IdxType _baseIdx;
        mutable IdxType _baseDiff;
    };
    
    class Element
    {
    public:
        
        Element(IdxType idx) :
        _idx(idx),
        _diff(1)
        { }
        
        Element(IdxType idx, IdxType diff) :
        _idx(idx),
        _diff(diff)
        { }
        
        Element() = default;
        
        //TODO needed?
        //Element(Element&&) = default;
        
        Element(IdxType idx, const Element&& element, const BaseElement& baseElement)
        {
            _idx = idx + baseElement.baseIdx();
            _diff = element._diff - baseElement.baseDiff();
        }
        
        void adjust() const
        {
            _idx--;
            _diff++;
        }
        
        void adjustIdx() const
        {
            _idx++;
        }
        
        IdxType idxImp(const BaseElement& baseElement) const
        {
            return _idx - baseElement.baseIdx();
        }
        
        IdxType diffImp(const BaseElement& baseElement) const
        {
            return _diff + baseElement.baseDiff();
        }
        
        IdxType adjustDiffImp(IdxType relIdx, const BaseElement& baseElement) const
        {
            return _diff += relIdx;
        }
        
        void setDiffImp(IdxType diff, const BaseElement& baseElement) const
        {
            _diff = diff - baseElement.baseDiff();
        }
        
        IdxType unbasedDiff() const {return _diff; }
        
    private:
        mutable IdxType _idx;
        mutable IdxType _diff;
    };
    
    template<class DDBaseLeafElement>
    static IdxType getDiff(DDBaseLeafElement& leafElement)
    {
        return leafElement->diffImp(*leafElement->basePtr());
    }
    
    template<class DDBaseLeafElement>
    static void setDiff(IdxType diff, DDBaseLeafElement& leafElement)
    {
        leafElement->setDiffImp(diff, *leafElement->basePtr());
    }
    
    template<class DDBaseLeafElement>
    static void adjustDiff(IdxType relDiff, DDBaseLeafElement& leafElement)
    {
        leafElement->adjustDiffImp(relDiff, *leafElement->basePtr());
    }
    //
    //
    
    
    
    
    
    
    
    
    
    
    
    
    
    /*
    class CompoundElement
    {
    public:
        
        CompoundElement(IdxType idx) :
        _idx(idx),
        _diff(1)
        { }
        
        IdxType idx() const
        {
            return _idx;
        }
        
        IdxType diff() const {return _diff; }
        
        //TODO hack because func is not const.
        void adjustDiff(IdxType relValue) const
        {
            _diff += relValue;
        }
        
        void setDiff(IdxType value) const
        {
            _diff = value;
        }
        
        void decrIdx() const
        {
            assert(_idx > 0);
            _idx--;
        }
        
        void incrIdx() const
        {
            _idx++;
        }
        
    private:
        mutable IdxType _idx;
        mutable IdxType _diff;
    };
    
    class Comparator
    {
        public:

        bool operator() (const CompoundElement& lhs, const CompoundElement& rhs) const
        {
            return lhs.idx() < rhs.idx();
        }
    };
    */
    
    //typedef std::set<CompoundElement, Comparator> DeleteContainer;
    typedef DDBaseSet<IdxType, Element, BaseElement, 40> DeleteContainer;
    
    
public:
    
    DDDeleteField() :
        fieldItr(*this)
    {}
    
    DDDeleteField(DDDeleteField&& other) :
        _set(std::forward<DeleteContainer>(other._set)),
        fieldItr(*this)
    {}
            
    void operator=(DDDeleteField<IdxType>&& rhs)
    {
        _set = std::forward<DeleteContainer>(rhs._set);
    }
            
    DDDeleteField(const DDDeleteField&) = delete;
    const DDDeleteField& operator=(const DDDeleteField&) = delete;
    
    void addIdx(IdxType idx)
    {
        /*
         
         Resolve The Triangle Case
        
            *       *
            | \  =  | \
            1  2    12 x
         
        */
        
        Element ce(idx);
        
        //auto biggerThanItr = std::upper_bound(_set.begin(), _set.end(), ce, Comparator());
        
        auto biggerThanItr = _set.upperBound(idx);
        
        //resolve triangle case
        bool nodeModified = false;
        bool hasRightNode = false;
        auto smallerEqualItr = biggerThanItr;
        
        //look for right triangle node.
        if (biggerThanItr->idx() > 0 && biggerThanItr->idx() - 1 == idx)
        {
            //ce.setDiff(biggerThanItr->diff() + 1);
            
            //**new ter
            ce = Element(idx, getDiff(biggerThanItr) + 1);
            
            _set.erase(biggerThanItr++);
            hasRightNode = true;
        
            smallerEqualItr = biggerThanItr;
        }
        
        //look for hit triangle node.
        if (biggerThanItr != _set.begin())
        {
            smallerEqualItr--;
            
            if (smallerEqualItr->idx() == idx)
            {
                /*
                if (hasRightNode) smallerEqualItr->setDiff(ce.diff());
                else smallerEqualItr->adjustDiff(1);
                */
                
                //**new
                if (hasRightNode) setDiff(ce.unbasedDiff(), smallerEqualItr);
                else adjustDiff(1, smallerEqualItr);
                
                _set.adjust(biggerThanItr);
                
                nodeModified = true;
            }
        }
        
    
        if (!nodeModified)
        {
            if (!hasRightNode)
            {
                IdxType lastDiff = 0;
                
                
                //if (biggerThanItr != _set.begin()) lastDiff = smallerEqualItr->diff();
                
                //**new
                if (biggerThanItr != _set.begin()) lastDiff = getDiff(smallerEqualItr);
                
                //ce.setDiff(lastDiff + 1);
            
                //**new ter
                ce = Element(idx, lastDiff + 1);
            }
            
            //insert the element. The iterator is only here to improve performance.
            //biggerThanItr = _set.insert(biggerThanItr, ce);
            //biggerThanItr++;
            
            //**new
            
            //insert(LeafSetPtr insertPtr, IdxType idx, const Element&& element)
            _set.insert(biggerThanItr, idx, std::move(ce));
            
        }
        
        //adjustIdxs(biggerThanItr);
    }
    
    IdxType adjustFieldAndEval(IdxType insertIdx)
    {
        //auto biggerThanItr = std::upper_bound(_set.begin(), _set.end(), CompoundElement(insertIdx), Comparator());
        
        //**new
        auto biggerThanItr = _set.upperBound(insertIdx);
        
        auto smallerEqualItr = biggerThanItr;
        
        if (biggerThanItr != _set.begin()) smallerEqualItr--;
        
        insertIdx = eval(insertIdx, smallerEqualItr);
        
        //***new
        _set.adjust(biggerThanItr, [](BaseElement&& baseElement){ baseElement.adjustIdx(); }, [](Element&& element){ element.adjustIdx(); });
        
        /*
        while (biggerThanItr != _set.end())
        {
            biggerThanItr->incrIdx();
            biggerThanItr++;
        }
        */
        
        return insertIdx;
    }
    
    //
    //iterator interface.
    typedef typename DeleteContainer::iterator BoundItr;
    DDFieldIterator<IdxType, DDDeleteField<IdxType>, Dummy> fieldItr;
    //
            
    IdxType eval(IdxType idx)
    {
        if (_set.size() > 0)
        {
            //auto itr = _set.upper_bound(CompoundElement(idx));
            
            //***new
            auto itr = _set.upperBound(idx);
            
            if (itr != _set.begin())
            {
                itr--;
                //idx += itr->diff();
                
                //***new
                idx += getDiff(itr);
            }
        }
        
        return idx;
    }
    
    void clear()
    {
        _set.clear();
    }

    std::vector<IdxType> allDeleteIdxs()
    {
        std::vector<IdxType> vec;
        IdxType lastDiff = 0;
        
        for (auto itr = _set.begin(); itr != _set.end(); itr++)
        {
            /*
            for (IdxType i=0; i<(itr->diff() - lastDiff); i++)
            {
                vec.push_back(itr->idx() + i + lastDiff);
            }
            
            lastDiff = itr->diff();
            */
            
            //***new
            for (IdxType i=0; i<(getDiff(itr) - lastDiff); i++)
            {
                vec.push_back(itr->idx() + i + lastDiff);
            }
            
            lastDiff = getDiff(itr);
        }
        
        return vec;
    }

    /*
    void debugPrint()
    {
        for (typename SetType::iterator itr = _set.begin(); itr != _set.end(); itr++)
        {
            std::cout << "_i_ " << itr->idx() << "_d_ " << itr->diff() << std::endl;
        }
    }
    */
    
private:
    DeleteContainer _set;
    
    /*
    void adjustIdxs(typename std::set<CompoundElement>::iterator& itr)
    {
        while (itr != _set.end())
        {
            itr->decrIdx();
            itr->adjustDiff(1);
            itr++;
        }
    }
    */
    
    //
    //iterator interface.
    friend class DDFieldIterator<IdxType, DDDeleteField<IdxType>, Dummy>;
            
    typename DeleteContainer::iterator beginItr()
    {
        return _set.begin();
    }
    
    IdxType eval(IdxType idx, typename DeleteContainer::iterator& itr)
    {
        while (itr != _set.end() && itr->idx() <= idx) itr++;
        
        assert(itr == _set.end() || itr->idx() > idx);
        
        auto tempItr = itr;
        
        //TODO performance!
        if (tempItr != _set.begin())
        {
            tempItr--;
            
            //idx += tempItr->diff();
        
            //***new
            idx += getDiff(tempItr);
        }
    
        return idx;
    }
    //
    //
};

#endif
