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
    
    DDActivePassivePtr(std::shared_ptr<PtrObj> activeObj, std::shared_ptr<PtrObj> passiveObj) :
        _activeObj(activeObj),
        _passiveObj(passiveObj)
    { }
    
    DDActivePassivePtr(const DDActivePassivePtr&) = delete;
    const DDActivePassivePtr& operator=(const DDActivePassivePtr&) = delete;
    
    std::shared_ptr<PtrObj> operator->() { return _activeObj; }
    
    std::shared_ptr<PtrObj>  back() { return _passiveObj; }
    
    void swap()
    {
        std::shared_ptr<PtrObj> temp = _activeObj;
        _activeObj = _passiveObj;
        _passiveObj = temp;
    }

private:
    std::shared_ptr<PtrObj> _activeObj;
    std::shared_ptr<PtrObj> _passiveObj;
};

#endif
