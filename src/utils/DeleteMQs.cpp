//
// Created by youh on 3/16/21.
//

#include <mqueue.h>
#include <iostream>
#include <cassert>
#include "../defs.h"
#include "../stl/String.h"

int main(int argc, char **argv) {
    if ( argc < 2 || scb::String(argv[1]) == "-h" || scb::String(argv[1]) == "--help" ) {
        std::cout << "Usage: ./delete-mq [mqueue-name(s)]" << std::endl;
        std::cout << "Where [mqueue-name(s)] can be one or more of the following queues seperated by a space:" << std::endl;
        std::cout << "  frontier" << std::endl;
        std::cout << "  newLinks" << std::endl;
        //TODO: Add more mqueues here as necessary, such as the index mqueue
        return 0;
    }

    for ( int i = 1; i < argc; ++i ) {
        mqd_t oldMqueue;
        const char *OLD_MQ_TO_UNLINK;
        // Delete the message queue listed
        if ( scb::String(argv[i]) == "frontier" ) {
            OLD_MQ_TO_UNLINK = FRONTIER_MQ;
        }
        else if ( scb::String(argv[i]) == "newLinks" ) {
            OLD_MQ_TO_UNLINK = NEW_LINKS_QUEUE;
        }
        else {
            std::cout << argv[i] << " is not a valid mqueue. Shutting down." << std::endl;
            return 0;
        }
        //TODO: Add more else ifs here as more mqueues are added

        oldMqueue = mq_open(OLD_MQ_TO_UNLINK, O_RDONLY | O_NONBLOCK);
        if ( oldMqueue == -1 ) {
            std::cout << "mq_open() failed for " << argv[1] << " since it may not have been created" << std::endl;
            continue;
        }

        char buf[MAX_URL_LEN + 1];
        int sz;
        while ((sz = mq_receive(oldMqueue, buf, MAX_URL_LEN, nullptr)) != -1) {
            buf[sz] = '\0';
            std::cout << "removed from " << argv[i] << ":  " << buf << std::endl;
        }


        // Detach the mqueue from this process and remove the mqueue from the system
        mq_close(oldMqueue);
        mq_unlink(OLD_MQ_TO_UNLINK);
        std::cout << argv[i] << " mqueue successfully deleted!" << std::endl;

    }
}
