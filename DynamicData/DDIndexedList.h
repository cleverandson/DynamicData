//
//  DDIndexedList.h
//  DynamicData
//
//  Created by mich2 on 3/16/13.
//  Copyright (c) 2013 -. All rights reserved.
//

#ifndef DynamicData_DDIndexedList_h
#define DynamicData_DDIndexedList_h

#include <list>

template<typename NodeType, typename IndexType>
class DDIndexedList
{
private:
    
    class NodeLink
    {
    public:
        void* upper;
        void* lower;
        IndexType offset;
    };
    
    typedef typename std::list<NodeLink> TopList;
    typedef typename TopList::iterator TopItr;
    
    class LeafNode
    {
    public:
        TopItr* upper;
        NodeType node;
        IndexType offset;
    };
    
    typedef typename std::list<LeafNode> LeafList;
    typedef typename LeafList::iterator LeafItr;
    
public:
    
    DDIndexedList() : _height(1) {}
    
    //TODO make private
    LeafItr getLowerLeafItr(IndexType index2, IndexType& offset)
    {
        IndexType newIndex = index2;
        //IndexType oldIndex;
        
        LeafItr beginLeafNodeItr = _leafList.begin();
        LeafItr endLeafNodeItr = _leafList.end();
        
        if (_height > 0)
        {
            LeafItr leafBegItr = _leafList.begin();
            //LeafItr leafEndItr = _leafList.end();
            
            TopItr hierarchyBegItr = _treeList.front().begin();
            TopItr hierarchyEndItr = _treeList.front().end();
            
            auto treeItr = _treeList.begin();
            
            size_t level = 0;
            
            bool shouldRun = true;
            
            while (shouldRun)
            {
                for (TopItr itr; itr != hierarchyEndItr; ++hierarchyBegItr)
                {
                    if (newIndex <= itr->offset)
                    {
                        if (level < _height - 1)
                        {
                            if (itr == treeItr->begin())
                            {
                                ++treeItr;
                                hierarchyBegItr = treeItr->begin();
                                --treeItr;
                            }
                            else
                            {
                                --itr;
                                
                                hierarchyBegItr = *(static_cast<TopItr*>(itr->lower));
                            }
                        }
                        else
                        {
                            shouldRun = false;
                            
                            leafBegItr = *(static_cast<LeafItr*>(itr->lower));
                            
                            ++itr;
                            
                            //set the end iterator if needed.
                            if (itr != hierarchyEndItr) beginLeafNodeItr = *(static_cast<LeafItr*>(itr->lower));
                        }
                        
                        break;
                    }
                    else
                    {
                        newIndex = newIndex - itr->offset;
                    }
                }
                
                if (!shouldRun) break;
                
                ++treeItr;
                hierarchyEndItr = treeItr->end();
                
                level++;
            }
            
        }
        
        LeafItr leafPtr = _leafList.end();
        
        for (auto itr = beginLeafNodeItr; itr != endLeafNodeItr; ++itr)
        {
            if (newIndex <= itr->offset)
            {
                if (itr == _leafList.begin())
                {
                    leafPtr = _leafList.begin();
                }
                else
                {
                    --itr;
                    leafPtr = itr;
                }
                
                break;
            }
            else
            {
                newIndex = newIndex - itr->offset;
            }
        }
        
        offset = newIndex;
        
        return leafPtr;
    }
    
    LeafItr get(IndexType index)
    {
        
    }
    
    bool insert(IndexType index, NodeType node2)
    {
        bool check = true;
        
        IndexType offset = 0;
        
        LeafItr lowerLeafItr = getLowerLeafItr(index, offset);
        
        //TODO handle special case begin of itr.
        
        if (offset == 0) check = false;
        else
        {
            LeafItr nextLeafItr = lowerLeafItr;
            nextLeafItr++;
            IndexType tempOffset = offset;
            tempOffset--;
            
            LeafItr endLeafItr = _leafList.end();
            
            LeafItr thisLeafItr;
            
            size_t segCount = 0;
            for (auto itr = nextLeafItr; itr != _leafList.end(); ++itr)
            {
                if (tempOffset == 0)
                {
                    thisLeafItr = itr;
                }
                
                if (itr->upper)
                {
                    endLeafItr = itr;
                    segCount++;
                    break;
                }
                
                tempOffset--;
                segCount++;
            }
            
            assert(offset < segCount);
            assert(segCount <= 2);
            
            if (segCount == 1)
            {
                
            }
            else
            {
                
                
            }
         
        }
        
        return check;
    }
    
private:
    size_t _height;
        
    std::list<TopList> _treeList;
    LeafList _leafList;
};

#endif
