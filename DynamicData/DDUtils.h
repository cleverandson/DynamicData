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

    static off_t fileSize(std::string filePath)
    {
        struct stat results;
        off_t size;
        
        if (stat(filePath.c_str(), &results) == 0)
        {
            size = results.st_size;
        }
        else
        {
            std::cout << "DDUtils: error in fileSize." << std::endl;
            exit(1);
        }
        
        return size;
    }
    
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
};

#endif
