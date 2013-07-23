#include "Server.h"
#include "Config.h"
#include "Log.h"
#include "Stratum/Server.h"

#include <boost/thread.hpp>
#include <iostream>

Server::Server() : serverLoops(0)
{
}

Server::~Server()
{
    //delete stratumServer;
}

int Server::Run()
{
	sLog.Info(LOG_SERVER, "Server is starting...");
	
    // Start stratum server
	sLog.Info(LOG_SERVER, "Starting stratum");
    //stratumServer = new Stratum::Server(Config::GetString("STRATUM_IP"), Config::GetInt("STRATUM_PORT"));
	
    // Init loop vars
    uint32_t sleepDuration = 0;
    int exitcode = 0;
    running = true;

    // Init diff
	uint32_t minDiffTime = sConfig.Get<uint32_t>("MinDiffTime");
    diffStart = boost::chrono::steady_clock::now();
	
	sLog.Info(LOG_SERVER, "Server is running!");

    while (running)
    {
        // Calc time diff
        boost::chrono::steady_clock::time_point now = boost::chrono::steady_clock::now();
        uint32_t diff = boost::chrono::duration_cast<boost::chrono::milliseconds>(now - diffStart).count();
        diffStart = now;

        // Update
        Update(diff);

        // Mercy for CPU
        if (diff < minDiffTime+sleepDuration) {
            sleepDuration = minDiffTime - diff + sleepDuration;
            boost::this_thread::sleep_for(boost::chrono::milliseconds(sleepDuration));
        } else
            sleepDuration = 0;

        ++serverLoops;

		if (serverLoops > 50)
			running = false;
        //std::cout << "Diff: " << diff << ", Loop: " << serverLoops << std::endl;
    }
    
    sLog.Info(LOG_SERVER, "Server is stopping...");

    return exitcode;
}

void Server::Update(uint32_t diff)
{

}
