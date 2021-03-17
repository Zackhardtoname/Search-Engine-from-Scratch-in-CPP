#include <iostream>
#include <cstdint>
#include <unistd.h>
#include <cstdlib>
#include <assert.h>
#include <mqueue.h>
#include <sys/wait.h>
#include <cstring>
#include <cassert>
#include "../../src/stl/include/Filesystem.h"
#include "../defs.h"

void seed(int argc) {
   // TODO question - can the attr be shared so to avoid recreating a ton of them...?
   // PubClient producer;
   struct mq_attr attr;
   attr.mq_flags = 0; // blocking
   attr.mq_maxmsg = 256;
   // Maximal url size most browsers support
   attr.mq_msgsize = MAX_URL_LEN;
   attr.mq_curmsgs = 0;
   /* create the message queue */
   mqd_t frontier = mq_open(FRONTIER_MQ, O_CREAT | O_RDWR, 0644, &attr);
   assert((mqd_t)-1 != frontier);

   char FileNameBuff[FILENAME_MAX];
   getcwd(FileNameBuff, FILENAME_MAX );
   string cwd (FileNameBuff);
   string seedPath = cwd + "/data/us_top_100.txt";
   const char * seedPathCStr = seedPath.c_str();
   char * seedStart;
   char * seedEnd;
   mapfile(&seedPathCStr, &seedStart, &seedEnd);
   char * seedFilePtr = seedStart;

   int curCrawlerNum = 0;

   while (seedFilePtr != seedEnd) {
      char URLBuff[MAX_URL_LEN];
      char * URLBuffPtr = URLBuff;

      while (*seedFilePtr != '\n') {
         *URLBuffPtr = *seedFilePtr;
         URLBuffPtr++;
         seedFilePtr++;
      }
      *URLBuffPtr = '\0';

      string message = string(URLBuff);
      if (message.substr(0, 4) != "http") {
         message = "http://www." + message + "\0";
      }

      assert(0 <= mq_send(frontier, message.c_str(), MAX_URL_LEN, 0));
      std::cout << "Queued message: " << message << std::endl;

      seedFilePtr++;
   }

   // cleanup
    assert((mqd_t)-1 != mq_close(frontier));
    assert((mqd_t)-1 != mq_unlink(FRONTIER_MQ));
}

int main(int argc, char *argv[]) {
   seed(argc);
   return 0;
}
