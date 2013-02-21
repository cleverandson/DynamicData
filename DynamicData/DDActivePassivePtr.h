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
