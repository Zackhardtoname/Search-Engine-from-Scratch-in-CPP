//
// Created by youh on 3/21/21.
//

#ifndef SCOOBLE_SEARCH_FILTERWORKER_H
#define SCOOBLE_SEARCH_FILTERWORKER_H

#include "../crawler/ParsedUrl.h"
#include "../indexer/IndexChunk.h"
#include "../stl/Pair.h"
#include "../stl/Queue.h"
#include "../stl/String.h"
#include "BloomFilter.h"
#include "Robotstxt.h"

#include <atomic>
#include <list>
#include <mqueue.h>
#include <pthread.h>
#include <semaphore.h>

using scb::Pair;
using scb::Queue;
using scb::String;
using scb::Vector;

/// Dataflow inside this worker:
///
/// newLinks mqueue --\ /-----> remoteUrls -----> other wk
///                    x
/// remote workers ---/ \----> filteredUrls ---> frontier
///
class FilterWorker
   {
      /// These threads are named by the data source they handle, declaration
      /// preceded by the resources they use.

      /// Thread that handles: newLinks mqueue -> ?
      mqd_t qNewLinks = -1;
      static const int NUM_NEWLINKS_THREAD = 5;
      pthread_t pthNewLinksMqueue[ NUM_NEWLINKS_THREAD ];

      struct FetchRobotstxtArgs
         {
            FilterWorker *self;
            ParsedUrl Url;
         };

      // Thread to fetch robots.txt and handle URLs pending on it
      // param [this thread takes ownership]: FetchRobotstxtArgs*
      static void *threadFetchRobotstxt( void * );

      // param: FilterWorker::this
      [[noreturn]] static void *threadNewLinksMqueue( void * );

      /// Thread that handles: remote workers -> ?
      // Host and port for other workers to post URLs.
      String hostname, port;
      pthread_t pthRemoteWorkers = -1;

      struct RemoteWorkerConnectionArgs
         {
            FilterWorker *self;
            int ConnFd;
         };

      // Handler thread for each remote worker connection
      // param [this thread takes ownership]: RemoteWorkerConnectionArgs*
      static void *threadRemoteWorkerConnection( void * );

      // Thread for waiting for connections from other remote workers
      // param: FilterWorker::this
      [[noreturn]] static void *threadRemoteWorkers( void * );

      /// Thread that handles: remoteUrls -> other workers
      int worldSize = -1, rank = -1;
      Vector<ParsedUrl> remoteWorkerAddrs;
      pthread_mutex_t lockRemoteWorkersSendBuffer{};
      Vector<String> remoteWorkersSendBuffer;
      int sendBufferAvailEventFd = -1;
      pthread_t pthRemoteUrls{};

      // Send the url to the remote worker (eventually)
      void queueRemoteUrl( int remote, const String &url );

      // Connect to an address and returns the sockfd.
      // Throws a runtime_exception if error.
      static int connectTo( const ParsedUrl &addr );

      // param: FilterWorker::this
      [[noreturn]] static void *threadRemoteUrls( void * );

      /// Thread that handles: filteredUrls -> frontier mqueue, with rate
      /// control
      // URLs that we have filtered, but have not gotten around to push into the
      // frontier mqueue.
      Queue<ParsedUrl> filteredUrls;
      pthread_mutex_t lockFilteredUrls{};
      mqd_t qFrontier = -1;
      pthread_t pthFilteredUrls{};

      // param: FilterWorker::this
      [[noreturn]] static void *threadFilteredUrls( void * );

      // this thread takes in a scb::Vector<ParsedUrl>, and offload them to a
      // file on disk.
      // param [this thread takes ownership]: Vector<ParsedUrl>* urls to offload
      static void *threadOffloadUrls( void * );

      /// A monitoring/stats thread
      std::atomic<long> count = 0;
      pthread_t pthMonitor;

      [[noreturn]] static void *threadMonitor( void * );

      /// /threads

      void handleUnfilteredLink( const String &link );

      // If the URL is not seen before, mark it seen.
      // Return true if it has already been seen before this operation.
      bool markIfNotSeen( const scb::String &url );

      // Get the maximum rate for a host per tick
      int getHostRateLimit( const String &host );

      pthread_mutex_t lockRobotstxtMap{};
      // Sentinels for a pending robots.txt (when many threads want to
      // download the same one at the same time), and a dummy robots.txt,
      // for when we cannot download/parse the robots.txt for a website.
      static const Robotstxt DummyRobotstxt;
      // TODO: use our own containers
      static const int MAX_NUM_ROBOTSTXT = 50000;
      struct LRUEntry
         {
            String Host;
            Robotstxt robotstxt;

            LRUEntry( String host, Robotstxt rb )
                : Host( std::move( host ) ), robotstxt( ( std::move( rb ) ) )
               {
               }
         };
      std::list<LRUEntry> robotstxtLRU;

      struct CacheInfo
         {
            bool Pending;
            // a list of urls waiting for this pending robots.txt
            Vector<ParsedUrl> WaitingURLs;
            decltype( robotstxtLRU )::iterator Pointer;
         };
      std::unordered_map<String, CacheInfo, StringHash> robotstxtMap;

      Bloomfilter bloomfilter;

      // block the whole process by acquiring all locks imaginable
      void block();

      // unblock the process
      void unblock();

   public:
      void Run();

      FilterWorker( const char *remoteConfigFile, int rank,
                    const char *bloomFile );
   };

#endif // SCOOBLE_SEARCH_FILTERWORKER_H
