#ifndef UTIL_H_
#define UTIL_H_

#include <sstream>
#include <iostream>
#include <queue>
#include <boost/date_time.hpp>
#include <boost/thread.hpp>
#include <boost/cstdint.hpp>

namespace Util
{
	std::string Date(const char* format, bool utc = false);
    
    template <typename T>
    class SynchronisedQueue
    {
    public:
        SynchronisedQueue() : __endQueue(false) {}
        
        void Enqueue(const T& data)
        {
            boost::unique_lock<boost::mutex> lock(__mutex);

            __queue.push(data);

            __cond.notify_one();
        }

        T Dequeue()
        {
            boost::unique_lock<boost::mutex> lock(__mutex);

            while (__queue.empty() && !__endQueue)
                __cond.wait(lock);
            
            if (__endQueue)
                return NULL;

            T result = __queue.front();
            __queue.pop();
            
            return result;
        }
        
        void Stop()
        {
            __endQueue = true;
            __cond.notify_all();        
        }
        
        uint32_t Size()
        {
            boost::unique_lock<boost::mutex> lock(__mutex);
            return __queue.size();
        }
        
    private:
        bool __endQueue;
        std::queue<T> __queue;
        boost::mutex __mutex;
        boost::condition_variable __cond;
    };
    
    class GenericWorker
    {
    public:
        void Activate()
        {
            _thread = new boost::thread(boost::bind(&GenericWorker::Work, this));
        }
    private:
        virtual void Work() = 0;
        boost::thread* _thread;
    };
}

#endif
