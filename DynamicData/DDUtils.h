//
//  DDUtils.h
//  DynamicData
//
//  Created by mich2 on 8/20/12.
//  Copyright (c) 2012 -. All rights reserved.
//

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
