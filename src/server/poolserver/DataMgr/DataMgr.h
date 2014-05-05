#ifndef DATAMGR_H_
#define DATAMGR_H_

#include <deque>
#include <boost/thread.hpp>
#include <boost/asio.hpp>

#include "Util.h"
#include "Share.h"
#include "Config.h"

using namespace boost;

class DataMgr
{
    // Singleton
private:
    static DataMgr* singleton;
public:
    
    static DataMgr* Instance()
    {
        return singleton;
    }
    
    static void Initialize(asio::io_service& io_service)
    {
        singleton = new DataMgr(io_service);
    }
    
public:
    DataMgr(asio::io_service& io_service) : _io_service(io_service), _uploadTimer(io_service)
    {
        UploadTimerStart();
    }
    
    void Push(Share data)
    {
        boost::unique_lock<boost::mutex> lock(_datamutex);
        _datastore.push_back(data);
    }
    
    Share Pop()
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
    
    void UploadTimerStart()
    {
        _uploadTimer.expires_from_now(boost::posix_time::seconds(sConfig.Get<uint32>("ShareUploadInterval")));
        _uploadTimer.async_wait(boost::bind(&DataMgr::UploadTimerExpired, this, boost::asio::placeholders::error));
    }

    void UploadTimerExpired(const boost::system::error_code& /*e*/)
    {
        Upload();
        UploadTimerStart();
    }
    
    void Upload();
private:
    // io service
    asio::io_service& _io_service;
    
    // timer
    boost::asio::deadline_timer _uploadTimer;
    
    boost::mutex _datamutex;
    std::deque<Share> _datastore;
};

#endif
