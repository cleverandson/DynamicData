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
    { }
    
    DDActivePassivePtr(DDActivePassivePtr&& other) :
        _activeObj(std::forward<PtrObj>(other._activeObj)),
        _passiveObj(std::forward<PtrObj>(other._passiveObj))
    { }
    
    void operator=(DDActivePassivePtr&& rhs)
    {
        _activeObj = std::forward<PtrObj>(rhs._activeObj);
        _passiveObj = std::forward<PtrObj>(rhs._passiveObj);
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
