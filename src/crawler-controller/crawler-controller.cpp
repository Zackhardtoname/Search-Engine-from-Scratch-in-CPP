#include <iostream>
#include <cstdint>
#include <unistd.h>
#include <cstdlib>
#include <assert.h>
#include <sys/wait.h>
#include "../stl/utils.h"
#include "../stl/RAII.h"
#include "../defs.h"

int main(int argc, char **argv) {
   if ( argc < 2 )
   {
      std::cerr << "Usage: ./crawler-controller child-process-file-path child-process-args\n";
      std::cerr << "It starts a number of child processes and makes sure they are alive\n";
      
      return 1;
   }

   std::cout << "crawler controller starting" << "\n";

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
