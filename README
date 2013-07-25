PoolServer
==========

Cryptocurrency mining pool written in C++ for speed. Supports Stratum.

Dependencies:
  Boost Libraries (http://www.boost.org/)
  MySQL Library (http://www.mysql.com/)
  CMake (http://www.cmake.org/)

Building on linux:
  # cd /path/to/source
  # mkdir build
  # cd build
  # cmake ../ -DPREFIX=/path/to/install
  # make
  # make install

Usage:

  -v [ --version ]                    print version string
  -h [ --help ]                       produce help message
  -c [ --config ] arg (=settings.cfg) name of a file of a configuration.

  --MinDiffTime arg (=100) Minimum server diff time

  -s [ --StratumHost ] arg (=0.0.0.0) Stratum server host
  -s [ --StratumPort ] arg (=3333)    Stratum server port

  --LogConsoleDebugMask arg (=0) Console log debug mask
  --LogFilePath arg (=.)         File log path
  --LogFileDebugMask arg (=0)    File log debug mask

  --DatabaseDriver arg (=mysql)     Database Driver

  --MySQLHost arg (=127.0.0.1)      MySQL Host
  --MySQLPort arg (=3306)           MySQL Port
  --MySQLUser arg (=root)           MySQL User
  --MySQLPass arg                   MySQL Password
  --MySQLDatabase arg (=poolserver) MySQL Database
  --MySQLSyncThreads arg (=2)       MySQL Sync Threads to Create
  --MySQLAsyncThreads arg (=2)      MySQL Async Threads to Create
