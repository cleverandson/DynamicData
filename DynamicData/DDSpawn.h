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
