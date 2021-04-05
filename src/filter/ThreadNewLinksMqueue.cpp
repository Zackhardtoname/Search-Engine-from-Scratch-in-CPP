//
// Created by youh on 4/3/21.
//

#include "FilterWorker.h"

void *FilterWorker::threadNewLinksMqueue( void *inst )
   {
   auto &self = *static_cast<FilterWorker *>( inst );

   char buf[ MAX_URL_LEN + 1 ];
   while ( true )
      {
      ssize_t sz = mq_receive( self.qNewLinks, buf, MAX_URL_LEN, nullptr );
      if ( sz == -1 )
         {
         std::cerr << "mq_receive() failed: " << strerror( errno ) << std::endl;
         exit( 1 );
         }

      buf[ sz ] = '\0';
      std::cout << "from local mq: " << buf << std::endl;

      pthread_mutex_lock( &self.lockUnfilteredUrls );
      String bufStr( buf );
      bufStr.shrinkToFit();
      self.unfilteredUrls.pushBack( std::move( bufStr ) );
      pthread_mutex_unlock( &self.lockUnfilteredUrls );

      // notify the downstream
      sem_post( &self.semUnfilteredUrls );
      }
   }
