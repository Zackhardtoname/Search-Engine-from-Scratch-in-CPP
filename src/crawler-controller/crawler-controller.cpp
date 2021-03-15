#include <iostream>
#include <cstdint>
#include <unistd.h>
#include <cstdlib>
#include <assert.h>
#include <sys/wait.h>
#include <cstring>
#include <vector>

const int mqMSGSize = 2048;
const int maxNumCharInUrl = 200;
const int priority = 0;
const int numCrawlers = 4; // TODO change to 2000 or something in production

using namespace std;

int main(int argc, char **argv) {
   std::vector<pid_t> processIdVec;

   pid_t processId = fork( );
   if ( processId ){
      // parent process
      int waitStatus;
      waitpid( processId, &waitStatus, 0 );
      cout << "child has exited with status = " << WEXITSTATUS( waitStatus )
           << endl;}
   else
   {
   // child process
      argv++;
      cout << "child starting " << *argv << endl;
      execvp( *argv, argv );
      cout << "this never prints" << endl;
   }

   return 0;
}
