#include <iostream>
#include <cstdint>
#include <unistd.h>
#include <cstdlib>
#include <assert.h>
#include <mqueue.h>
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

void seed() {
   // PubClient producer;
   struct mq_attr attr;
   attr.mq_flags = 0; // blocking
   attr.mq_maxmsg = 200;
   // Maximal url size most browsers support
   attr.mq_msgsize = mqMSGSize;
   attr.mq_curmsgs = 0;

   /* create the message queue */
   auto mq = mq_open("url", O_CREAT | O_WRONLY, 0644, &attr);
   CHECK((mqd_t)-1 != mq);

   char FileNameBuff[FILENAME_MAX];
   getcwd(FileNameBuff, FILENAME_MAX );
   string cwd (FileNameBuff);
   string seedPath = cwd + "/data/us_top_100.txt";
   const char * seedPathCStr = seedPath.c_str();
   char * seedStart;
   char * seedEnd;
   mapfile(&seedPathCStr, &seedStart, &seedEnd);
   char * seedFilePtr = seedStart;

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

      CHECK(0 <= mq_send(mq, message.c_str(), mqMSGSize, priority));
      std::cout << "Queued message: " << message << std::endl;

      seedFilePtr++;
   }
}

int main() {
   seed();
   return 0;
}
