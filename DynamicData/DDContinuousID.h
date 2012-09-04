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
                
    static IdType bound()
    {
        static IdType bound = (IdType)-1 / 2;
        return bound;
    }
                
    IdType value()
    {
        return _value;
    }
                
private:
    IdType _value;
};

#endif
