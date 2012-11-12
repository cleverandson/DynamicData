//
//  DDRandomGen.h
//  DynamicData
//
//  Created by mich2 on 11/12/12.
//  Copyright (c) 2012 -. All rights reserved.
//

#ifndef DynamicData_DDRandomGen_h
#define DynamicData_DDRandomGen_h

template<typename ValueType>
class DDRandomGen
{
public:
    
    DDRandomGen() : _distribution(0, std::numeric_limits<ValueType>::max()) {}
    
    DDRandomGen(size_t startRange, size_t endRange) : _distribution(startRange, endRange) {}
    
    ValueType randVal() { return _distribution(DDRandomGen::engine()); }
    
private:
    std::uniform_int_distribution<ValueType> _distribution;
    
    static std::mt19937& engine()
    {
        static std::mt19937 engine;
        return engine;
    }
};

#endif
