#include <iostream>
#include <cstdint>
#include <unistd.h>
#include <cstdlib>
#include <assert.h>
#include <sys/wait.h>
#include <mqueue.h>
#include <signal.h>
#include "../stl/utils.h"
#include "../stl/RAII.h"
#include "../defs.h"

mqd_t frontier;
mqd_t newLinks;

void signal_callback_handler(int signum) {
   std::cout << "\nCaught signal " << signum << std::endl;
   std::cout << "Parsing stopped, shutting down each worker and the parent process." << std::endl;
   
   // Cleanup each mqueue
   assert((mqd_t)-1 != mq_close(frontier));
   assert((mqd_t)-1 != mq_unlink(FRONTIER_MQ));
   assert((mqd_t)-1 != mq_close(newLinks));
   assert((mqd_t)-1 != mq_unlink(NEW_LINKS_QUEUE));

   // Terminate program
   exit(signum);
}

int main(int argc, char **argv) {
   if ( argc < 2 )
   {
      std::cerr << "Usage: ./crawler-controller child-process-file-path child-process-args\n";
      std::cerr << "It starts a number of child processes and makes sure they are alive\n";
      
      return 1;
   }


   // Register signal and signal handler
   signal(SIGINT, signal_callback_handler);

   std::cout << "crawler controller starting" << "\n";

   struct mq_attr attr;
   char buffer[MAX_URL_LEN + 1];
   char answer = 'Y';

   // initialize the queue attributes
   // TODO: Figure out how to set attr.mq_maxmsg to a higher value
   attr.mq_flags = 0;
   attr.mq_maxmsg = 256;
   attr.mq_msgsize = MAX_URL_LEN;
   attr.mq_curmsgs = 0;

   // Create the frontier message queue
   frontier = mq_open(FRONTIER_MQ, O_CREAT | O_RDONLY, 0644, &attr);
   assert((mqd_t)-1 != frontier);

   // Create the new link message queue
   // TODO: Reconsider if we want to use a different attr variable for
   // the frontier queue verse the newLinks queue. Changing it would
   // mean changing the max size and max message size of the newLink
   // queue verse the frontier.
   newLinks = mq_open(NEW_LINKS_QUEUE, O_CREAT | O_RDONLY, 0644, &attr);
   assert((mqd_t)-1 != newLinks);

   scb::Vector<pid_t> processIdVec;
   processIdVec.resize(NUM_CRAWLERS);
   char * processFileName = argv[1];
   bool initalLoop = true;

   while (true) {
      for (int i = 0; i < NUM_CRAWLERS; ++i) {
         int waitStatus;

         if (!initalLoop) {
            // check if child process is alive
            if (waitpid(processIdVec[i], &waitStatus, WNOHANG) == 0) {
               continue;
            }
            std::cout << "child has exited with status = " << WEXITSTATUS( waitStatus ) << std::endl;
         }
         
         pid_t processId = fork( );

         if (processId) {
            // parent process
            processIdVec[i] = processId;
         }
         else
         {
            // child process
            argv++; // get rid of filename of the controller; optionally we can change it to the filename of the child file
            std::cout << "starting crawler " << i << std::endl;
            execvp( processFileName, argv );
            // Don't add code here because it will never run
         }         
      }

      initalLoop = false;
   }

   return 0;
}
