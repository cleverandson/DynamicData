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


#ifndef DynamicData_DDMMapAllocator_h
#define DynamicData_DDMMapAllocator_h

#include <map>

#include <fcntl.h>
#include <sys/mman.h>
#include <sstream>
#include <thread>

#include "MMapWrapper.h"
#include "DDUtils.h"
#include "DDFileHandle.h"

template<typename IdxType, typename Type, class UserDataHeader> using MMapWrapperPtr = std::unique_ptr<MMapWrapper<IdxType, Type, UserDataHeader>>;

template<typename IdxType>
class DDMMapAllocator
{
    typedef std::string FilePath;
    
public:
    
    DDMMapAllocator()
    {
        system("mkdir -p data");
    }
    
    DDMMapAllocator(const DDMMapAllocator&) = delete;
    const DDMMapAllocator& operator=(const DDMMapAllocator&) = delete;

    template<typename Type, class UserDataHeader>
    MMapWrapperPtr<IdxType, Type, UserDataHeader> getHandleFromDataStore(size_t scopeVal, size_t idVal)
    {
        std::pair<size_t,size_t> tuple = std::make_tuple(scopeVal, idVal);
        
        if (!_persistentIds.count(tuple))
        {
            _persistentIds[tuple] = true;
            
            DDFileHandle fileId(scopeVal, idVal, [this, tuple](){ deleteFileId(tuple); });
            //TODO adjust padding size!
            return MMapWrapperPtr<IdxType, Type, UserDataHeader>(new MMapWrapper<IdxType, Type, UserDataHeader>(std::move(fileId),/* paddingSize! */ 4096));
        }
        else
        {
            std::cout << "DDMMapAllocator: error scope " << scopeVal << " id " << idVal << " in use!" << std::endl;
            exit(1);
        }
    }
        
    static DDMMapAllocator* SHARED()
    {
        return DDUtils::SHARED<DDMMapAllocator>();
    }
    
private:
    
    std::map<std::pair<size_t,size_t>, bool> _persistentIds;
    
    void deleteFileId(std::pair<size_t,size_t> fileIdIN)
    {
        size_t erased = _persistentIds.erase(fileIdIN);
        assert(erased);
        
        std::string sysMsg("rm -r ");
        sysMsg.append(DDFileHandle::path(std::get<0>(fileIdIN), std::get<1>(fileIdIN)));
        system(sysMsg.c_str());
    }
};

#endif
