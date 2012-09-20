//
//  DDContinuousID.h
//  DynamicData
//
//  Created by mich2 on 8/21/12.
//  Copyright (c) 2012 -. All rights reserved.
//

#ifndef DynamicData_DDContinuousID_h
#define DynamicData_DDContinuousID_h

//unsigned type needed here!
template<typename IdType>
class DDContinuousID
{
public:
    
    DDContinuousID() : _value(0) {}
    
    DDContinuousID(IdType value) : _value(value) {}
    
    bool operator< (const DDContinuousID& b) const
    {
        bool check = abs(b._value - _value) < bound();
        
        if (check) return _value < b._value;
        else return _value > b._value;
    }
    
    bool operator> (const DDContinuousID& b) const
    {
        bool check = abs(b._value - _value) < bound();
        
        if (check) return _value > b._value;
            else return _value < b._value;
    }
    
    //TODO test this!
    void incr()
    {
        _value++;
    }
          
    //test this.
    IdType diff(const DDContinuousID& b)
    {
        IdType diff;
        
        if (b._value > _value && this > b)
        {
            diff = b._value - _value;
        }
        else if (b._value > _value && this < b)
        {
            diff = maxId() - b._value + _value;
        }
        else if (b._value < _value && this > b)
        {
            diff = _value - b._value;
        }
        else if (b._value < _value && this < b)
        {
            diff = maxId() - _value + b._value;
        }
        
        return diff;
    }
                
    static IdType bound()
    {
        static IdType bound = (IdType)-1 / 2;
        return bound;
    }
             
    //test this.
    static IdType maxId()
    {
        return (IdType)-1;
    }
                
    IdType value()
    {
        return _value;
    }
                
private:
    IdType _value;
};

#endif
