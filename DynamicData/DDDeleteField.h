/*
 
    This file is part of DynamicData.

    DynamicData is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Foobar is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Foobar.  If not, see <http://www.gnu.org/licenses/>.
 
*/

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
    
    typedef std::set<CompoundElement, Comparator> DeleteContainer;
    //typedef std::vector<CompoundElement> DeleteContainer;
    
            
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
        
        CompoundElement ce(idx);
        
        //auto biggerThanItr = std::upper_bound(_set.begin(), _set.end(), ce, Comparator());
        auto biggerThanItr = _set.upper_bound(ce);
        
        //resolve triangle case
        bool nodeModified = false;
        bool hasRightNode = false;
        auto smallerEqualItr = biggerThanItr;
        
        
        
        //look for right triangle node.
        if (biggerThanItr != _set.end() && biggerThanItr->idx() > 0 && biggerThanItr->idx() - 1 == idx)
        {
            ce.setDiff(biggerThanItr->diff() + 1);
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
                if (hasRightNode) smallerEqualItr->setDiff(ce.diff());
                else smallerEqualItr->adjustDiff(1);
                
                nodeModified = true;
            }
        }
        
    
        if (!nodeModified)
        {
            if (!hasRightNode)
            {
                IdxType lastDiff = 0;
                if (biggerThanItr != _set.begin()) lastDiff = smallerEqualItr->diff();
                    
                ce.setDiff(lastDiff + 1);
            }
            
            //insert the element. The iterator is only here to improve performance.
            biggerThanItr = _set.insert(biggerThanItr, ce);
            biggerThanItr++;
        }
        
        adjustIdxs(biggerThanItr);
    }
    
    IdxType adjustFieldAndEval(IdxType insertIdx)
    {
        //auto biggerThanItr = std::upper_bound(_set.begin(), _set.end(), CompoundElement(insertIdx), Comparator());
        auto biggerThanItr = _set.upper_bound(CompoundElement(insertIdx));
        
        auto smallerEqualItr = biggerThanItr;
        
        if (biggerThanItr != _set.begin()) smallerEqualItr--;
        
        insertIdx = eval(insertIdx, smallerEqualItr);
        
        while (biggerThanItr != _set.end())
        {
            biggerThanItr->incrIdx();
            biggerThanItr++;
        }
        
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
            auto itr = _set.upper_bound(CompoundElement(idx));
            //auto itr = std::upper_bound(_set.begin(), _set.end(), CompoundElement(idx), Comparator());
            
            if (itr != _set.begin())
            {
                itr--;
                idx += itr->diff();
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
            for (IdxType i=0; i<(itr->diff() - lastDiff); i++)
            {
                vec.push_back(itr->idx() + i + lastDiff);
            }
            
            lastDiff = itr->diff();
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
    //TODO rename set.
    DeleteContainer _set;
    
    void adjustIdxs(typename DeleteContainer::iterator& itr)
    {
        while (itr != _set.end())
        {
            itr->decrIdx();
            itr->adjustDiff(1);
            itr++;
        }
    }
    
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
            idx += tempItr->diff();
        }
    
        return idx;
    }
    //
    //
};

#endif
