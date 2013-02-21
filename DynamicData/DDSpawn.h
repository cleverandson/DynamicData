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

#ifndef DynamicData_DDSpawn_h
#define DynamicData_DDSpawn_h

class DDSpawn
{
public:
    
    DDSpawn(size_t maxThreads) :
        _maxThreads(maxThreads),
        _threadCount(0)
    {
        
    }
    
    DDSpawn(const DDSpawn&) = delete;
    const DDSpawn& operator=(const DDSpawn&) = delete;
    
    void joinAllThreads()
    {
        waitUntilThreadCountSmallerThen(1);
    }
    
    void runAsyncBlocking(std::function<void ()> func)
    {
        waitUntilThreadCountSmallerThen(_maxThreads);
        
        std::unique_lock<std::mutex> lock(_mutex);
        _threadCount++;
        
        std::thread t(&DDSpawn::asyncRun, this, func);
        t.detach();
    }
    
private:
    std::mutex _mutex;
    size_t _maxThreads;
    int _threadCount;
    std::condition_variable _condVar;
    
    void asyncRun(std::function<void ()> func)
    {
        func();
        
        std::unique_lock<std::mutex> lock(_mutex);
        _threadCount--;
        
        _condVar.notify_all();
    }
    
    void waitUntilThreadCountSmallerThen(size_t value)
    {
        while(true)
        {
            std::unique_lock<std::mutex> lock(_mutex);
            
            if (_threadCount >= value)
            {
                _condVar.wait(lock);
                
                if (_threadCount < value)
                {
                    break;
                }
            }
            else
            {
                break;
            }
        }
    }
};

#endif
