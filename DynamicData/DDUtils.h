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
