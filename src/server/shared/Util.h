#ifndef UTIL_H_
#define UTIL_H_

#include "Common.h"

#include <sstream>
#include <iostream>
#include <queue>
#include <string>
#include <boost/date_time.hpp>
#include <boost/thread.hpp>
#include <boost/archive/iterators/base64_from_binary.hpp>
#include <boost/archive/iterators/binary_from_base64.hpp>
#include <boost/archive/iterators/transform_width.hpp>
#include <boost/archive/iterators/insert_linebreaks.hpp>
#include <boost/archive/iterators/remove_whitespace.hpp>
#include <boost/algorithm/string.hpp>

namespace Util
{
    std::string Date(const char* format, bool utc = false);
    uint32 Date();
    std::string FS(const char *str, ...);
    std::vector<std::string> Explode(std::string input, std::string delim);
    
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
        ~GenericWorker()
        {
            delete _thread;
        }
        
        void Activate()
        {
            _thread = new boost::thread(boost::bind(&GenericWorker::Work, this));
        }
    private:
        virtual void Work() = 0;
        boost::thread* _thread;
    };
    
    // Base64 encode/decode functions by http://stackoverflow.com/users/1132850/piquer
    
    typedef boost::archive::iterators::insert_linebreaks
            <
                boost::archive::iterators::base64_from_binary
                <
                    boost::archive::iterators::transform_width<std::string::const_iterator, 6, 8>
                >, 72
            > it_base64_lb_t;
    
    typedef boost::archive::iterators::base64_from_binary
            <
                boost::archive::iterators::transform_width<std::string::const_iterator, 6, 8>
            > it_base64_t;
    
    typedef boost::archive::iterators::transform_width
            <
                boost::archive::iterators::binary_from_base64<std::string::const_iterator>, 8, 6
            > it_binary_t;
    
    std::string ToBase64(std::string input, bool linebreaks = true);
    std::string FromBase64(std::string input);
    
    uint8 ASCIIToHex(char ch);
    std::vector<byte> ASCIIToBin(std::string str);
    std::string BinToASCII(std::vector<byte> data);
    std::vector<byte> Reverse(std::vector<byte> data);
    std::vector<byte> Join(std::vector<byte> v1, std::vector<byte> v2);
}

#endif
