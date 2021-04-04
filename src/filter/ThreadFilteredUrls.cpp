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

void *FilterWorker::threadOffloadUrls( void *p )
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
   bool blocked = false; // if mqueue is full and blocks us
   while ( !gotSIGINT.load() )
      {
      pthread_mutex_lock( &self.lockFilteredUrls );

      // offload some urls to disk if we have too many in memory
      if ( self.filteredUrls.size() >
           PENDING_LINKS_OFFLOAD_FACTOR * MAX_PENDING_LINKS )
         {
         // make a vector of URLs to offload
         auto *offload = new Vector<ParsedUrl>;
         while ( self.filteredUrls.size() > MAX_PENDING_LINKS )
            {
            offload->pushBack( std::move( self.filteredUrls.front() ) );
            self.filteredUrls.popFront();
            }

         // spawn a new thread to offload the URLs to not block this thread
         pthread_t th;
         if ( pthread_create( &th, nullptr, threadOffloadUrls, offload ) != 0 )
            {
            // if new thread failed, just do the offloading here
            threadOffloadUrls( offload );
            delete offload;
            }
         else
            pthread_detach( th );
         }

      // count how many times have we hit this host in this tick
      Counter hostCounter;

      for ( int i = 0, loop = self.filteredUrls.size();  i < loop;  i++ )
         {
         ParsedUrl url = std::move( self.filteredUrls.front() );
         self.filteredUrls.popFront();

         // this should not happen, but check just in case
         if ( url.CompleteUrl.size() > MAX_URL_LEN )
            {
            std::cout << "URL too long: " << url.CompleteUrl << std::endl;
            continue;
            }

         // if rate limit for this host reached, requeue
         if ( hostCounter.Get( url.Host ) >= self.getHostRateLimit( url.Host ) )
            {
            self.filteredUrls.pushBack( url );
            std::cout << "rate limit hit for: " << url.CompleteUrl << std::endl;
            continue;
            }

         // if mqueue blocked, wait until the next tick
         if ( mq_send( self.qFrontier, url.CompleteUrl.data(),
                       url.CompleteUrl.size(), 0 ) == -1 )
            {
            if ( errno == EAGAIN )
               {
               self.filteredUrls.pushBack( url ); // requeue
               // log only if not previously blocked, instead of every tick
               if ( !blocked )
                  std::cout
                     << "frontier mqueue blocked: crawling isn't fast enough"
                     << std::endl;
               blocked = true;
               break; // wait until the next tick
               }
            else
               assert( false && "mq_send() failed" );
            }

         blocked = false;

         // update rate count
         hostCounter.Increment( url.Host );

         std::cout << "posted to frontier: " << url.CompleteUrl << std::endl;
         }

      pthread_mutex_unlock( &self.lockFilteredUrls );

      usleep( 200 * 1000 ); // wait until next tick
      }

   // graceful exit

   // has to be a pointer because we transfer ownership to the offload function
   auto *save = new Vector<ParsedUrl>;
   while ( !self.filteredUrls.empty() )
      {
      save->pushBack( std::move( self.filteredUrls.front() ) );
      self.filteredUrls.popFront();
      }

   threadOffloadUrls( save );

   // kill my other brothers as well
   exit( 0 );
   }
