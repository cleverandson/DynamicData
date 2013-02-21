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


#ifndef DynamicData_DDFieldIterator_h
#define DynamicData_DDFieldIterator_h

//TODO make two classes for the two fields.
template<typename IdxType, class Field, class CachedElement>
class DDFieldIterator
{
public:
    
    DDFieldIterator(Field& field) :
        _field(field),
        _currIdx(0)
    {}
    
    virtual ~DDFieldIterator() {}
    
    DDFieldIterator(const DDFieldIterator&) = delete;
    const DDFieldIterator& operator=(const DDFieldIterator&) = delete;
    
    
    void startItr()
    {
        _boundaryItr = _field.beginItr();
        _currIdx = 0;
    }
    
    IdxType itrEvalAndStep()
    {
        IdxType res = _field.eval(_currIdx, _boundaryItr);
        _currIdx++;
        
        return res;
    }
    
    /*
    //TODO remove?
    IdxType itrEvalAndStep(bool& hasCacheElement, CachedElement& cachedElement)
    {
        IdxType res = _field.eval(_currIdx, _boundaryItr, hasCacheElement, cachedElement);
        _currIdx++;
        
        return res;
    }
    */
    
    //TODO check this!
    IdxType itrEval(IdxType idx, bool& hasCacheElement, CachedElement& cachedElement)
    {
        if (idx < _currIdx) assert(0);
        _currIdx = idx;
        
        IdxType res = _field.eval(_currIdx, _boundaryItr, hasCacheElement, cachedElement);
        //_currIdx++;
        
        return res;
    }
    
    void reset()
    {
        _currIdx = 0;
    }
    
private:
    Field& _field;
    IdxType _currIdx;
    typename Field::BoundItr _boundaryItr;
};

#endif
