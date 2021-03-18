//
// Created by epfeiffe on 3/18/21.
//

#include <mqueue.h>
#include <iostream>
#include <cassert>
#include <mqueue.h>
#include "../defs.h"
#include "../stl/String.h"

int main(int argc, char **argv) {
    if ( argc < 2 || scb::String(argv[1]) == "-h" || scb::String(argv[1]) == "--help" ) {
        std::cout << "Usage: ./create-mq [mqueue-name(s)]" << std::endl;
        std::cout << "Where [mqueue-name(s)] can be one or more of the following queues seperated by a space:" << std::endl;
        std::cout << "  frontier" << std::endl;
        std::cout << "  newLinks" << std::endl;
        //TODO: Add more mqueues here as necessary, such as the index mqueue
        return 0;
    }

    for ( int i = 1; i < argc; ++i ) {

        mqd_t newMqueue;
        const char *NEW_MQ;
        struct mq_attr attr;
        char buffer[MAX_URL_LEN + 1];
        // initialize the queue attributes
        // TODO: Figure out how to set attr.mq_maxmsg to a higher value
        // TODO: Consider tweaking these attributes for each individual queue type
        // For example, we may want the frontier to have a size of 1000 but newLinks
        // to only have a size of 500
        attr.mq_flags = 0;
        attr.mq_maxmsg = 256;
        attr.mq_msgsize = MAX_URL_LEN;
        attr.mq_curmsgs = 0;

        // Create the message queue listed
        if ( scb::String(argv[i]) == "frontier" ) {
            NEW_MQ = FRONTIER_MQ;
        }
        else if ( scb::String(argv[i]) == "newLinks" ) {
            NEW_MQ = NEW_LINKS_QUEUE;
        }
        else {
            std::cout << argv[i] << " is not a valid mqueue. Shutting down." << std::endl;
            return 0;
        }
        //TODO: Add more else ifs here as more mqueues are added

        if ( mq_open(NEW_MQ, O_RDONLY | O_NONBLOCK) != -1 ) {
            std::cout << argv[1] << " mqueue already exsists, skipping for now" << std::endl;
            continue;
        }
        newMqueue = mq_open(NEW_MQ, O_CREAT | O_RDONLY, 0644, &attr);

        assert(newMqueue != -1 && "mq_open() failed");
        // Detach the newly created mqueue from this process
        mq_close(newMqueue);
        std::cout << argv[i] << " mqueue successfully created!" << std::endl;

    }
}
