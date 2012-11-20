//
//  MMapWrapper.h
//  DynamicData
//
//  Created by mich2 on 8/19/12.
//  Copyright (c) 2012 -. All rights reserved.
//

#ifndef DynamicData_MMapWrapper_h
#define DynamicData_MMapWrapper_h

#include <unistd.h>
#include <assert.h>

#include "DDUtils.h"
#include "DDFileHandle.h"

template<typename IdxType, class Type, class UserDataHeader>
class MMapWrapper
{
public:
    
    class HeaderData
    {
    public:
        unsigned long long date;
        IdxType mapSize;
    };
    
    MMapWrapper(DDFileHandle&& ddFileHandle, IdxType paddingSize) :
        _mapSize(0),
        _fileSize(0),
        _ddFileHandle(std::move(ddFileHandle)),
        _paddingSize(paddingSize),
        _triplePaddingSize(paddingSize * 3),
        _headerSize(sizeof(HeaderData) + sizeof(UserDataHeader)),
        _isMapped(false),
        _userDataHeaderPtr(0)
    {
        _fileDesc = open(_ddFileHandle.path().c_str(), O_RDWR | O_CREAT, (mode_t)0600);
        off_t rawFileSize = _ddFileHandle.fileSize();
        
        //create a new file.
        if (rawFileSize == 0)
        {
            relResizeFile(2);
            writeMapSizeToFile();
            
            assert(_userDataHeaderPtr);
            _userDataHeaderPtr[0] = UserDataHeader();
        }
        //use existing file.
        else
        {
            _fileSize = (rawFileSize - _headerSize) / sizeof(Type);
            map();
            
            //assing the mapSize.
            HeaderData headerData = ((HeaderData*)_rawMap)[0];
            _mapSize = headerData.mapSize;
            
            assert(_fileSize >= _mapSize);
        }
    }
    
    ~MMapWrapper()
    {
        unmap();
        close(_fileDesc);
    }
    
    MMapWrapper(const MMapWrapper&) = delete;
    const MMapWrapper& operator=(const MMapWrapper&) = delete;
    
    void unpersist()
    {
        _ddFileHandle.unpersist();
    }
    
    Type getVal(IdxType idx)
    {
        assert(idx < _mapSize);
        
        return _map[idx];
    }
    
    void persistVal(IdxType idx, Type value)
    {
        assert(idx < _fileSize);
        
        _map[idx] = value;
        
        if (idx >= _mapSize) _mapSize = idx + 1;
        
        remapIfNeeded2();
        writeMapSizeToFile();
    }
    
    //TODO remove this.
    void deleteIdx(IdxType idx, std::function<void (IdxType oldIdx, IdxType remapIdx)> remapFunc)
    {
        assert(idx < _mapSize);
        
        IdxType endIdx = _mapSize-1;
        
        _map[idx] = _map[endIdx];
        _mapSize--;
        
        remapFunc(endIdx, idx);
        
        remapIfNeeded2();
        writeMapSizeToFile();
    }
    
    //TODO new test this. replacement for shrinkSize.
    void resize(IdxType size)
    {
        _mapSize = size;
        remapIfNeeded2();
    }
    
    IdxType size()
    {
        return _mapSize;
    }
    
    IdxType fileSize()
    {
        return _fileSize;
    }
    
    UserDataHeader getUserDataHeader()
    {
        assert(_userDataHeaderPtr);
        
        return _userDataHeaderPtr[0];
    }
    
    void saveUserDataHeader(const UserDataHeader userDataHeader)
    {
        char* tempPtr = _rawMap;
        tempPtr += sizeof(HeaderData);
        
        UserDataHeader* ptr = (UserDataHeader*)tempPtr;
        
        ptr[0] = userDataHeader;
    }
    
private:
    Type* _map;
    char* _rawMap;
    int _fileDesc;
    
    IdxType _mapSize;
    IdxType _fileSize;
    DDFileHandle _ddFileHandle;
    IdxType _paddingSize;
    IdxType _triplePaddingSize;
    IdxType _headerSize;
    UserDataHeader _userDataHeader;
    
    UserDataHeader* _userDataHeaderPtr;
    
    bool _isMapped;
    
    void unmap()
    {
        if(_isMapped)
        {
            if (munmap(_rawMap, _fileSize * sizeof(Type) + _headerSize) == -1)
            {
                //TODO abstract the error logs.
                std::cout << "MMapWrapper: error un-mmapping file " << _mapSize << std::endl;
                exit(1);
            }
        
            _isMapped = false;
        }
    }
    
    void map()
    {
        if (!_isMapped && _fileSize > 0)
        {
            _rawMap = (char*)mmap(0, _fileSize * sizeof(Type) + _headerSize, PROT_READ | PROT_WRITE, MAP_SHARED, _fileDesc, 0);
            
            //assing the map pointer (strip off header)
            char* tempPtr = _rawMap;
            tempPtr += sizeof(HeaderData);
            _userDataHeaderPtr = (UserDataHeader*)tempPtr;
            
            tempPtr += sizeof(UserDataHeader);
            _map = (Type*)tempPtr;
            
            _isMapped = true;
        }
    }
    
    void remapIfNeeded2()
    {
        IdxType diff = _fileSize - _mapSize;
        
        if (diff < _paddingSize) relResizeFile(2);
        else if (diff > _triplePaddingSize) resizeFile(_mapSize);
         
    }
    
    //TODO rename.
    void relResizeFile(int delta)
    {
        unmap();
        if (delta > 0) _fileSize += (delta * _paddingSize);
        else _fileSize -= (-delta * _paddingSize);
        
        ftruncate(_fileDesc, _fileSize * sizeof(Type) + _headerSize);
        
        map();
    }
    
    void resizeFile(IdxType size)
    {
        unmap();
        
        _fileSize = size + 2 * _paddingSize;
        ftruncate(_fileDesc, _fileSize * sizeof(Type) + _headerSize);
        
        map();
    }
    
    void writeMapSizeToFile()
    {
        HeaderData headerData;
        headerData.mapSize = _mapSize;
        
        HeaderData* tempPtr = (HeaderData*)_rawMap;
        tempPtr[0] = headerData;
    }
};

#endif
