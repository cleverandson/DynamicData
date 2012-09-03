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

#include "MMapWrapper.h"
#include "DDUtils.h"

template<typename IdxType>
class DDMMapAllocator
{
public:
    
    DDMMapAllocator()
    {
        system("mkdir -p data");
        //system("mkdir -p data");
    }
    
    DDMMapAllocator(const DDMMapAllocator&) = delete;
    const DDMMapAllocator& operator=(const DDMMapAllocator&) = delete;

    template<typename Type, class UserDataHeader>
    std::unique_ptr<MMapWrapper<IdxType, Type, UserDataHeader>> getHandleFromDataStore(size_t scopeVal, size_t idVal)
    {
        std::tuple<size_t,size_t> tuple = std::make_tuple(scopeVal, idVal);
        
        if(!_persistentIds.count(tuple))
        {
            _persistentIds[tuple] = true;
        
            std::stringstream file;
            file << "data/data_scope_" << scopeVal << "_id_" << idVal << ".bin";
        
            //TODO adjust padding size!
            return std::unique_ptr<MMapWrapper<IdxType, Type, UserDataHeader>>(new MMapWrapper<IdxType, Type, UserDataHeader>(file.str(),/* paddingSize */ 128));
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
    std::map<std::tuple<size_t,size_t>, bool> _persistentIds;
    std::map<std::tuple<size_t,size_t>, bool> _indexIds;
};

#endif
