//
// Created by youh on 4/3/21.
//

#include <csignal>
#include <ctime>
#include <fstream>
#include <unistd.h>

#include "../stl/Counter.h"
#include "../stl/utils.h"
#include "FilterWorker.h"

void *FilterWorker::threadOffloadFilteredUrls( void *p )
   {
   // cast and relegate the argument pointer to a smart pointer
   scb::UniquePointer urls( static_cast<Vector<ParsedUrl> *>( p ) );

   // get current time
   time_t timestamp = std::time( nullptr );
   tm *timeRepr = std::gmtime( &timestamp );
   char timeStr[ 1000 ];
   strftime( timeStr, sizeof( timeStr ), "%Y_%m_%d_%H_%M_%S_", timeRepr );

   // write to a file named like 2021_03_22_15_30_21_random.urls.txt
   scb::String filename =
      scb::String( timeStr ) + RandAlphaString( 6 ) + ".urls.txt";
   std::fstream fs( filename.cstr(), std::ios::out | std::ios::app );

   std::cout << "offloading " << urls->size() << " URLs to " << filename
             << std::endl;
   for ( ParsedUrl &url : *urls )
      fs << url.CompleteUrl << '\n';

   return nullptr;
   }

std::atomic<bool> gotSIGINT = false;
void SIGINTHandler( int )
   {
   gotSIGINT.store( true );
   }

void *FilterWorker::threadFilteredUrls( void *inst )
   {
   // unmask SIGINT such that only this thread can get that signal
   sigset_t sigset;
   sigemptyset( &sigset );
   sigaddset( &sigset, SIGINT );
   pthread_sigmask( SIG_UNBLOCK, &sigset, nullptr );

   // set handler for SIGINT so that we stop the loop on SIGINT
   struct sigaction sigactionINT
      {
      };
   sigactionINT.sa_handler = SIGINTHandler;
   sigactionINT.sa_flags = 0;
   sigaction( SIGINT, &sigactionINT, nullptr );

   // actual work
   auto &self = *static_cast<FilterWorker *>( inst );
   while ( !gotSIGINT.load() )
      {
      pthread_mutex_lock( &self.lockFilteredUrls );

      // count how many times have we hit this host in this tick
      Counter hostCounter;

      for ( int i = 0, loop = self.filteredUrls.size();  i < loop;  i++ )
         {
         ParsedUrl url = std::move( self.filteredUrls.front() );
         self.filteredUrls.popFront();

         // this should not happen, but check just in case
         if ( url.CompleteUrl.size() > MAX_URL_LEN )
            {
#ifdef VERY_VERBOSE
            std::cout << "URL too long: " << url.CompleteUrl << std::endl;
#endif
            continue;
            }

         // if rate limit for this host reached, requeue
         if ( hostCounter.Get( url.Host ) >= self.getHostRateLimit( url.Host ) )
            {
            self.filteredUrls.pushBack( url );
            continue;
            }

         // if mqueue blocked, wait until the next tick
         if ( mq_send( self.qFrontier, url.CompleteUrl.data(),
                       url.CompleteUrl.size(), 0 ) == -1 )
            {
            if ( errno == EAGAIN )
               {
               self.filteredUrls.pushBack( url ); // requeue
               break;                             // wait until the next tick
               }
            else
               assert( false && "mq_send() failed" );
            }

         // update rate count
         hostCounter.Increment( url.Host );

#ifdef VERY_VERBOSE
         std::cout << "posted to frontier: " << url.CompleteUrl << std::endl;
#endif
         }

      pthread_mutex_unlock( &self.lockFilteredUrls );

      usleep( 200 * 1000 ); // wait until next tick
      }

   // graceful exit
   pthread_mutex_lock( &self.lockFilteredUrls );

   if ( self.filteredUrls.empty() )
      std::cout << "no URLs pending, skipping offloading" << std::endl;
   else
      {
      // has to be a pointer because we transfer ownership to the offload
      // function
      auto *save = new Vector<ParsedUrl>;
      while ( !self.filteredUrls.empty() )
         {
         save->pushBack( std::move( self.filteredUrls.front() ) );
         self.filteredUrls.popFront();
         }

      threadOffloadFilteredUrls( save );
      }

   // kill my other brothers as well
   std::cout << "gracefully exiting" << std::endl;
   exit( 0 );
   }
