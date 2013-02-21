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
