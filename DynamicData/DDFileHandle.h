//
//  DDFileHandle.h
//  DynamicData
//
//  Created by mich2 on 11/8/12.
//  Copyright (c) 2012 -. All rights reserved.
//

#ifndef DynamicData_DDFileHandle_h
#define DynamicData_DDFileHandle_h

//TODO expand this class to a general file handel class. all the disk access should be handled form this class!
//TODO make path customizable.
class DDFileHandle
{
public:
    
    DDFileHandle(size_t scopeVal, size_t idVal, std::function<void ()> unpersist) :
    _scopeVal(scopeVal),
    _idVal(idVal),
    _unpersist(unpersist)
    {}
    
    DDFileHandle(const DDFileHandle&& other) :
    _scopeVal(other._scopeVal),
    _idVal(other._idVal),
    _unpersist(other._unpersist)
    {}
    
    void operator=(DDFileHandle&& rhs)
    {
        _scopeVal = rhs._scopeVal;
        _idVal = rhs._idVal;
        _unpersist = rhs._unpersist;
    }
    
    DDFileHandle(const DDFileHandle& other) = delete;
    const DDFileHandle& operator=(const DDFileHandle&) = delete;
    
    std::string path() const
    {
        return DDFileHandle::path(_scopeVal, _idVal);
    }
    
    static std::string path(size_t scope, size_t idVal)
    {
        std::stringstream file;
        file << "data/data_scope_" << scope << "_id_" << idVal << ".bin";
        return file.str();
    }
    
    void unpersist()
    {
        _unpersist();
    }
    
    off_t fileSize()
    {
        struct stat results;
        off_t size;
        
        if (stat(path().c_str(), &results) == 0) size = results.st_size;
        else
        {
            std::cout << "DDUtils: error in fileSize." << std::endl;
            exit(1);
        }
        
        return size;
    }
    
private:
    size_t _scopeVal;
    size_t _idVal;
    std::function<void ()> _unpersist;
};

#endif
