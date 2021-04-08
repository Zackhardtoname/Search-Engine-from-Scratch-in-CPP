//
// Created by youh on 4/6/21.
//

#include <unistd.h>

#include "FilterWorker.h"

[[noreturn]] void *FilterWorker::threadMonitor( void *inst )
   {
   auto &self = *static_cast<FilterWorker *>( inst );
   long prev = 0;
   while ( true )
      {
      sleep( 10 );
      long cur = self.count.load();
      std::cout << "rate (link/s): " << ( cur - prev ) / 10.0 << std::endl;
      prev = cur;
      }
   }
