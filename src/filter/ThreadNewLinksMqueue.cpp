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
      mq_attr frontierAttr{};
      assert( mq_getattr( self.qFrontier, &frontierAttr ) == 0 );

      if ( frontierAttr.mq_curmsgs >= 0.9 * frontierAttr.mq_maxmsg )
         usleep( 200 * 1000 );

      ssize_t sz = mq_receive( self.qNewLinks, buf, MAX_URL_LEN, nullptr );
      if ( sz == -1 )
         {
         std::cerr << "mq_receive() failed: " << strerror( errno ) << std::endl;
         exit( 1 );
         }

      String url( buf, sz );
#ifdef VERY_VERBOSE
      std::cout << "from local mq: " << url << std::endl;
#endif

      self.handleUnfilteredLink( url );
      }
   }
