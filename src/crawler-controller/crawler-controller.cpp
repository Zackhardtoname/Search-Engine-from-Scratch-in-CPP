#include <iostream>
#include <cstdint>
#include <unistd.h>
#include <cstdlib>
#include <assert.h>
#include <sys/wait.h>
#include <cstring>
#include <vector>
#include "../defs.h"

using namespace std;

int main(int argc, char **argv) {
   if ( !( ( argc >= 2 ) ) )
   {
      std::cerr << "Usage: ./crawler-controller child-process-file-path child-process-args\n";
      std::cerr << "It starts a number of child processes and makes sure they are alive\n";
   }

   std::cout << "crawler controller starting" << "\n";

   std::vector<pid_t> processIdVec;
   char * processFileName = argv[1];
   bool initalLoop = true;

   while (true) {
      for (int i = 0; i < NUM_CRAWLERS; ++i) {
         int waitStatus;

         if (!initalLoop) {
            // timed wait for child process
            bool isAlive = false;
            while(waitpid(processIdVec[i], &waitStatus, WNOHANG) == 0) {
               sleep(1);
               isAlive = true;
            }
            if (isAlive) {
               continue;
            }

            
            cout << "child has exited with status = " << WEXITSTATUS( waitStatus ) << endl;
         }
         
         pid_t processId = fork( );

         if (processId) {
            // parent process
            if (initalLoop) {
               processIdVec.push_back(processId);
            }
            else {
               processIdVec[i] = processId;
            }
         }
         else
         {
            // child process
            argv++; // get rid of filename of the controller; optionally we can change it to the filename of the child file
            cout << "starting crawler " << i << endl;
            execvp( processFileName, argv );
            // Don't add code here because it will never run
         }         
      }

      initalLoop = false;
   }

   return 0;
}
