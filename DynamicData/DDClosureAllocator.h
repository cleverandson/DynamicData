//
//  DDClosureAllocator.h
//  DynamicData
//
//  Created by mich2 on 8/21/12.
//  Copyright (c) 2012 -. All rights reserved.
//

#ifndef DynamicData_DDClosureAllocator_h
#define DynamicData_DDClosureAllocator_h



template<typename IdxType>
class DDClosureAllocator
{
public:

    class ClosureParams
    {
    public:
        size_t type;
        size_t idVal;
        int arg1;
        int arg2;
    };
    
    DDClosureAllocator()
    {
        
    }
    
    DDClosureAllocator(const DDClosureAllocator&) = delete;
    const DDClosureAllocator& operator=(const DDClosureAllocator&) = delete;


    template<size_t type>
    std::function<IdxType (IdxType inIdx)> getClosure(int arg1, int arg2)
    {
        
    }
    
    
    static DDClosureAllocator<IdxType>* SHARED()
    {
        return DDUtils::SHARED<DDClosureAllocator<IdxType>>();
    }
};

#endif
