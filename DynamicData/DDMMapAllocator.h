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
