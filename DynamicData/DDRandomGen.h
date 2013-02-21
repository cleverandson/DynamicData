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

#ifndef DynamicData_DDRandomGen_h
#define DynamicData_DDRandomGen_h

#include <random>

template<typename ValueType>
class DDRandomGen
{
public:
    
    DDRandomGen() : _distribution(0, std::numeric_limits<ValueType>::max()) {}
    
    DDRandomGen(ValueType startRange, ValueType endRange) : _distribution(startRange, endRange) {}
    
    ValueType randVal() { return _distribution(DDRandomGen::engine()); }
    
private:
    std::uniform_int_distribution<ValueType> _distribution;
    
    static std::mt19937& engine()
    {
        static std::random_device rd;
        static std::mt19937 engine(rd());
        return engine;
    }
};

#endif
