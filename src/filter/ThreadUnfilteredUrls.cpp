//
// Created by youh on 4/3/21.
//

#include "FilterWorker.h"

[[noreturn]] void *FilterWorker::threadUnfilteredUrls( void *inst )
   {
   auto &self = *static_cast<FilterWorker *>( inst );

   while ( true )
      {
      // wait until the queue has stuff in it
      sem_wait( &self.semUnfilteredUrls );

      pthread_mutex_lock( &self.lockUnfilteredUrls );
      ParsedUrl url( self.unfilteredUrls.front() );
      self.unfilteredUrls.popFront();
      pthread_mutex_unlock( &self.lockUnfilteredUrls );

      // filter by domain hash
      int rightfulHandler = url.Host.hash() % self.worldSize;
      if ( rightfulHandler != self.rank )
         {
         std::cout << "not our host: " << url.Host << std::endl;
         self.queueRemoteUrl( rightfulHandler, url.CompleteUrl );
         continue;
         }

      // filter by robots.txt
      if ( !self.allowedByRobotstxt( url ) )
         {
         std::cout << "not allowed by robots.txt: " << url.CompleteUrl
                   << std::endl;
         continue;
         }

      // filter by bloom filter
      if ( self.markIfNotSeen( url.CompleteUrl ) )
         {
         std::cout << "seen: " << url.CompleteUrl << std::endl;
         continue;
         }

      // all good! we can put that into filteredUrls
      pthread_mutex_lock( &self.lockFilteredUrls );
      self.filteredUrls.pushBack( std::move( url ) );
      pthread_mutex_unlock( &self.lockFilteredUrls );

      // notify the downstream
      sem_post( &self.semFilteredUrls );
      }
   }
