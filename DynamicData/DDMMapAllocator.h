//
//  DDMMapAllocator.h
//  DynamicData 
//
//  Created by mich mich on 8/16/12.
//  Copyright (c) 2012 -. All rights reserved.
//

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
            return MMapWrapperPtr<IdxType, Type, UserDataHeader>(new MMapWrapper<IdxType, Type, UserDataHeader>(std::move(fileId),/* paddingSize! */ 128));
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
