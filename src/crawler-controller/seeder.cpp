#include <iostream>
#include <cstdint>
#include <unistd.h>
#include <cstdlib>
#include <assert.h>
#include <mqueue.h>
#include <sys/wait.h>
#include <cstring>
#include "../../src/stl/include/Filesystem.h"

// #include "String.h"

// CHECK macro from https://stackoverflow.com/questions/3056307/how-do-i-use-mqueue-in-a-c-program-on-a-linux-based-system
#define CHECK(x) \
    do { \
        if (!(x)) { \
            fprintf(stderr, "%s:%d: ", __func__, __LINE__); \
            perror(#x); \
            exit(-1); \
        } \
    } while (0)  \

const int mqMSGSize = 2048;
const int maxNumCharInUrl = 200;
const int priority = 0;
const int numCrawlers = 4; // TODO change to 2000 or something in production

void seed() {
   vector<mqd_t> urlQueues;

   for (int i = 0; i < numCrawlers; ++i) {
      // TODO question - can the attr be shared so to avoid recreating a ton of them...?
      // PubClient producer;
      struct mq_attr attr;
      attr.mq_flags = 0; // blocking
      attr.mq_maxmsg = 200;
      // Maximal url size most browsers support
      attr.mq_msgsize = mqMSGSize;
      attr.mq_curmsgs = 0;
      /* create the message queue */
      mqd_t mq = mq_open("url", O_CREAT | O_WRONLY, 0644, &attr);
      // CHECK((mqd_t)-1 != mq);
      urlQueues.push_back(mq);
   }

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
      char URLBuff[maxNumCharInUrl];
      char * URLBuffPtr = URLBuff;

      while (*seedFilePtr != '\n') {
         *URLBuffPtr = *seedFilePtr;
         URLBuffPtr++;
         seedFilePtr++;
      }
      *URLBuffPtr = '\0';

      string message = string(URLBuff);
      if (message.substr(0, 4) != "http") {
         message = "http://www." + message + "\n";
      }

      CHECK(0 <= mq_send(urlQueues[curCrawlerNum++], message.c_str(), mqMSGSize, priority));
      std::cout << "Queued message: " << message << std::endl;

      seedFilePtr++;
      curCrawlerNum %= numCrawlers;
   }
}

int main() {
   seed();
   return 0;
}
