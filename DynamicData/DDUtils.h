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

#ifndef DynamicData_DDUtils_h
#define DynamicData_DDUtils_h

#include <sys/stat.h>
#include "DDFileHandle.h"

static void PrintOUT(std::string line){ std::cout << "OUT:" << line << std::endl; }

#ifdef DEBUG
static void PrintDBUG(std::string className, std::string line){ std::cout << className << ": " << line << std::endl; }
#else
#define PrintDBUG(X)
#endif

static void PrintERROR(std::string line){ std::cout << "ERROR :" << line << std::endl; }



class DDUtils
{
public:

    DDUtils(const DDUtils&) = delete;
    const DDUtils& operator=(const DDUtils&) = delete;

    template <typename TYPE>
    static TYPE* SHARED()
    {
        static TYPE* _type = 0;
        
        if (!_type)
        {
            _type = new TYPE();
        }
        
        return _type;
    }
    
    template<typename T, typename ...Args>
    static std::unique_ptr<T> make_unique( Args&& ...args )
    {
        return std::unique_ptr<T>( new T( std::forward<Args>(args)... ) );
    }
};

#endif
