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

#include <mqueue.h>
#include <pthread.h>
#include <semaphore.h>

// TODO remove
#include "../stl/StringHash.h"

using scb::Pair;
using scb::Queue;
using scb::String;
using scb::Vector;

/// Dataflow inside this worker:
///
/// newLinks mqueue --v                      -----> remoteUrls -----> other wk
///                   ---> unfilteredUrls ---^v
/// remote workers ---^                       ----> filteredUrls ---> frontier
///
class FilterWorker
   {
      /// These threads are named by the data source they handle, declaration
      /// preceded by the resources they use.

      /// Thread that handles: newLinks mqueue -> unfilteredUrls
      mqd_t qNewLinks;
      pthread_t pthNewLinksMqueue;

      [[noreturn]] static void *threadNewLinksMqueue( void * );

      /// Thread that handles: remote workers -> unfilteredUrls
      // Host and port for other workers to post URLs.
      String hostname, port;
      pthread_t pthRemoteWorkers;

      struct RemoteWorkerConnectionArgs
         {
            FilterWorker *self;
            int connFd;
         };

      static void *threadRemoteWorkerConnection( void * );

      [[noreturn]] static void *threadRemoteWorkers( void * );

      /// Thread that handles: unfilteredUrls -> ( remoteUrls | filteredUrls )
      static constexpr int NUM_FILTER_THREADS = 10;
      Queue<String> unfilteredUrls; // URLs waiting to be filtered.
      pthread_mutex_t lockUnfilteredUrls;
      sem_t semUnfilteredUrls;
      pthread_t pthUnfilteredUrls[ NUM_FILTER_THREADS ];

      [[noreturn]] static void *threadUnfilteredUrls( void * );

      /// Thread that handles: remoteUrls -> other workers
      int worldSize, rank;
      Vector<ParsedUrl> remoteWorkerAddrs;
      pthread_mutex_t lockRemoteWorkersSendBuffer;
      Vector<String> remoteWorkersSendBuffer;
      pthread_t pthRemoteUrls;

      void queueRemoteUrl( int remote, const String &s );

      static int connectTo( const ParsedUrl &addr );

      [[noreturn]] static void *threadRemoteUrls( void * );

      /// Thread that handles: filteredUrls -> frontier mqueue, with rate
      /// control
      // URLs that we have filtered, but have not gotten around to push into the
      // frontier mqueue.
      Queue<ParsedUrl> filteredUrls;
      pthread_mutex_t lockFilteredUrls;
      sem_t semFilteredUrls;
      mqd_t qFrontier;
      pthread_t pthFilteredUrls;

      [[noreturn]] static void *threadFilteredUrls( void * );

      // If the URL is not seen before, mark it seen.
      // Return true if it has already been seen before this operation.
      bool markIfNotSeen( const scb::String &url );

      // Get the maximum rate for a host per tick
      int getHostRateLimit( const String &host );

      pthread_mutex_t lockRobotstxtMap;
      static Robotstxt *const ROBOTS_TXT_PENDING, *const DummyRobotstxt;
      std::unordered_map<scb::String, Robotstxt *, StringHash>
         robotstxtMap; // TODO: use our own hashmap
      bool allowedByRobotstxt( const ParsedUrl &url );

      Bloomfilter bloomfilter;

   public:
      void run();

      FilterWorker( const char *remoteConfigFile, int rank,
                    const char *bloomFile );
   };

#endif // SCOOBLE_SEARCH_FILTERWORKER_H
