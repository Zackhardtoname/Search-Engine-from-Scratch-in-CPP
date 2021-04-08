//
// Created by youh on 4/8/21.
//

#include "../stl/utils.h"
#include "FilterWorker.h"
#include <fstream>

void *FilterWorker::threadOffloadUnfilteredUrls( void *p )
   {
   auto *self = static_cast<FilterWorker *>( p );

   int numCurrentDump = NUM_OFFLOAD_UNFILTERED_PER_DUMP;
   std::fstream fs;

   while ( true )
      {
      mq_attr frontierAttr{};
      assert( mq_getattr( self->qFrontier, &frontierAttr ) == 0 );

      mq_attr newLinksAttr{};
      assert( mq_getattr( self->qNewLinks, &newLinksAttr ) == 0 );

      if ( newLinksAttr.mq_curmsgs >= newLinksAttr.mq_maxmsg * FULL_FACTOR )
         {
         for ( int i = 0;  i < NUM_OFFLOAD_UNFILTERED_PER_BURST;  i++ )
            {
            if ( numCurrentDump++ >= NUM_OFFLOAD_UNFILTERED_PER_DUMP )
               {
               // switch to the next dump

               // get current time
               time_t timestamp = std::time( nullptr );
               tm *timeRepr = std::gmtime( &timestamp );
               char timeStr[ 1000 ];
               strftime( timeStr, sizeof( timeStr ), "%Y_%m_%d_%H_%M_%S_",
                         timeRepr );

               // write to a file named like
               // 2021_03_22_15_30_21_random.unfiltered.txt
               scb::String filename = scb::String( timeStr ) +
                                      RandAlphaString( 6 ) + ".unfiltered.txt";
               fs.close();
               std::cout << "new unfiltered offload: " << filename << std::endl;
               fs = std::fstream( filename.cstr(),
                                  std::ios::out | std::ios::app );
               numCurrentDump = 0;
               }

            // get one from newLinks and write to file
            char buf[ MAX_URL_LEN + 1 ];
            auto sz = mq_receive( self->qNewLinks, buf, sizeof( buf ), nullptr );
            buf[ sz ] = '\0';
            fs << buf << '\n';
            }
         continue; // check again
         }
      usleep( 0.5 * 1000 );
      }
   }
