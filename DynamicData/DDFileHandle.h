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
