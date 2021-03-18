//
// Created by epfeiffe on 3/18/21.
//

#include <mqueue.h>
#include <iostream>
#include <cassert>
#include "../defs.h"
#include "../stl/String.h"

int main(int argc, char **argv) {
    if ( argc < 2 || scb::String(argv[1]) == "-h" || scb::String(argv[1]) == "--help" ) {
        std::cout << "Usage: ./check-mq [mqueue-name(s)]" << std::endl;
        std::cout << "Where [mqueue-name(s)] can be one or more of the following queues seperated by a space:" << std::endl;
        std::cout << "  frontier" << std::endl;
        std::cout << "  newLinks" << std::endl;
        //TODO: Add more mqueues here as necessary, such as the index mqueue
        return 0;
    }

    for ( int i = 1; i < argc; ++i ) {
        mqd_t checkMqueue;
        const char *MQ_TO_CHECK;
        struct mq_attr attr;
        // Delete the message queue listed
        if ( scb::String(argv[i]) == "frontier" ) {
            MQ_TO_CHECK = FRONTIER_MQ;
        }
        else if ( scb::String(argv[i]) == "newLinks" ) {
            MQ_TO_CHECK = NEW_LINKS_QUEUE;
        }
        else {
            std::cout << argv[i] << " is not a valid mqueue. Shutting down." << std::endl;
            return 0;
        }
        //TODO: Add more else ifs here as more mqueues are added

        checkMqueue = mq_open(MQ_TO_CHECK, O_RDONLY | O_NONBLOCK, 0644, &attr);
        if ( checkMqueue == -1 ) {
            std::cout << "mq_open() failed for " << argv[1] << " since it may not have been created" << std::endl;
            continue;
        }

        struct mq_attr *cur_attr = &attr;
        mq_getattr(checkMqueue, cur_attr);
        assert((mqd_t)-1 != checkMqueue);
        assert(cur_attr != nullptr);
        std::cout << argv[1] << " mqueue has " << cur_attr->mq_curmsgs << " elements" << std::endl;


        // Detach the mqueue from this process
        mq_close(checkMqueue);

    }
}
