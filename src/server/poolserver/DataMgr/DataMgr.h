#ifndef DATAMGR_H_
#define DATAMGR_H_

#include <deque>
#include <boost/thread.hpp>

#include "Util.h"
#include "Share.h"

#define BULK_MIN 1
#define BULK_COUNT 50

template<class T>
class DataMgr
{
public:
    DataMgr() {}
    
    void Push(T data)
    {
        boost::unique_lock<boost::mutex> lock(_datamutex);
        _datastore.push_back(data);
    }
    
    T Pop()
    {
        boost::unique_lock<boost::mutex> lock(_datamutex);
        Share share = _datastore.front();
        _datastore.pop_front();
        return share;
    }
    
    size_t Size()
    {
        boost::unique_lock<boost::mutex> lock(_datamutex);
        return _datastore.size();
    }
    
    void Upload();
private:
    boost::mutex _datamutex;
    std::deque<T> _datastore;
};

extern DataMgr<Share> sDataMgr;

#endif
