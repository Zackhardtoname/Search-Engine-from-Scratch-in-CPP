//
// Created by youh on 4/6/21.
//

#include <unistd.h>

#include "FilterWorker.h"

[[noreturn]] void *FilterWorker::threadMonitor( void *inst )
   {
   auto &self = *static_cast<FilterWorker *>( inst );
   long prevFlowin = 0, prevFlowout = 0;
   while ( true )
      {
      sleep( MONITOR_INTERVAL_SEC );
      long curFlowin = self.flowinCount.load(),
           curFlowout = self.flowoutCount.load();
      std::cout << "rate (link/s) in: "
                << ( curFlowin - prevFlowin ) / MONITOR_INTERVAL_SEC << " out: "
                << ( curFlowout - prevFlowout ) / MONITOR_INTERVAL_SEC
                << std::endl;
      prevFlowin = curFlowin;
      prevFlowout = curFlowout;
      }
   }
