#include <iostream>
#include <cstdint>
#include <unistd.h>
#include <cstdlib>
#include <assert.h>
#include <sys/wait.h>
#include "../stl/utils.h"
#include "../stl/RAII.h"
#include "../defs.h"

bool interruptReceived = false;

void SIGINTHandler(int) {
    interruptReceived = true;
}

void SIGTERMHandler(int) {
    // so that we do not get killed by killpg() and can die after reaping our children.
}

int main(int argc, char **argv) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " [child-process-file-path] [child-process-args]\n";
        std::cerr << "This watchdog starts a number of child processes and makes sure they are alive\n";

        return 1;
    }
    char *childExecPath = argv[1];
    signal(SIGINT, SIGINTHandler);
    signal(SIGTERM, SIGTERMHandler);

    std::cout << "crawler controller starting" << std::endl;
    // spin-up NUM_CRAWLERS crawlers
    for (int i = 0; i < NUM_CRAWLERS; ++i)
        if (pid_t child = fork()) {
            std::cout << "started child " << child << std::endl;
        } else
            // 5.1.2.2.1 Program startup ... -- argv[argc] shall be a null pointer.
            execvp(childExecPath, ++argv);

    // watch for death of children and restart them
    int childStatus;
    while (pid_t deadChild = wait(&childStatus)) {
        if (deadChild == -1) {
            std::cout << "wait(): " << strerror(errno) << std::endl;
            return 255;
        }

        if (interruptReceived) {
            std::cout << "interrupt received... killing all children" << std::endl;
            killpg(0, SIGTERM);
            while (wait(nullptr) != -1);
            return 0;
        }

        std::cout << "child pid = " << deadChild << " exited status = " << WEXITSTATUS(childStatus) << std::endl;
        if (pid_t child = fork()) {
            std::cout << "started child " << child << std::endl;
        } else
            // 5.1.2.2.1 Program startup ... -- argv[argc] shall be a null pointer.
            execvp(childExecPath, ++argv);
    }

    return 0;
}
