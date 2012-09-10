//
//  DDSpawn.h
//  DynamicData
//
//  Created by mich2 on 9/6/12.
//  Copyright (c) 2012 -. All rights reserved.
//

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
