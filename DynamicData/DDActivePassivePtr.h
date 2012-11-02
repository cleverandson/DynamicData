//
//  DDActivePassivePtr.h
//  DynamicData
//
//  Created by mich2 on 10/29/12.
//  Copyright (c) 2012 -. All rights reserved.
//

#ifndef DynamicData_DDActivePassivePtr_h
#define DynamicData_DDActivePassivePtr_h

template<class PtrObj>
class DDActivePassivePtr
{
public:
    
    //disable none rvalue construction.
    DDActivePassivePtr(PtrObj& activeObj, PtrObj& passiveObj) = delete;
    
    DDActivePassivePtr(PtrObj&& activeObj, PtrObj&& passiveObj) :
        _activeObj(std::forward<PtrObj>(activeObj)),
        _passiveObj(std::forward<PtrObj>(passiveObj))
    {
    
    }
    
    DDActivePassivePtr(const DDActivePassivePtr&) = delete;
    const DDActivePassivePtr& operator=(const DDActivePassivePtr&) = delete;
    
    PtrObj* operator->() { return &_activeObj; }
    
    PtrObj&  back() { return _passiveObj; }
    
    void swap()
    {
        PtrObj temp(std::move(_activeObj));
        _activeObj = std::move(_passiveObj);
        _passiveObj = std::move(temp);
    }

private:
    PtrObj _activeObj;
    PtrObj _passiveObj;
};

#endif
