//
// Created by epfeiffe on 3/18/21.
//

#include <mqueue.h>
#include <iostream>
#include <cassert>
#include <mqueue.h>
#include "../defs.h"
#include "../stl/String.h"


void handleMqueue(int argc, char **argv, scb::String operation) {
    for ( int i = 2; i < argc; ++i ) {
        mqd_t mqueue;
        const char *MQUEUE_NAME;
        // Delete the message queue listed
        if ( scb::String(argv[i]) == "frontier" ) {
            MQUEUE_NAME = FRONTIER_MQ;
        }
        else if ( scb::String(argv[i]) == "newLinks" ) {
            MQUEUE_NAME = NEW_LINKS_QUEUE;
        }
        else {
            std::cout << argv[i] << " is not a valid mqueue. Shutting down." << std::endl;
            return;
        }
        //TODO: Add more else ifs here as more mqueues are added

        
        if ( operation == "create" ) {
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

            if ( mq_open(MQUEUE_NAME, O_RDONLY | O_NONBLOCK) != -1 ) {
                std::cout << argv[i] << " mqueue already exsists, skipping for now" << std::endl;
                continue;
            }
            mqueue = mq_open(MQUEUE_NAME, O_CREAT | O_RDONLY, 0644, &attr);
            assert(mqueue != -1 && "mq_open() failed");
            std::cout << argv[i] << " mqueue successfully created!" << std::endl;

        }
        else if ( operation == "check" ) {
            struct mq_attr attr;

            mqueue = mq_open(MQUEUE_NAME, O_RDONLY | O_NONBLOCK, 0644, &attr);
            if ( mqueue == -1 ) {
                std::cout << "mq_open() failed for " << argv[i] << " since it may not have been created" << std::endl;
                continue;
            }

            struct mq_attr *cur_attr = &attr;
            mq_getattr(mqueue, cur_attr);
            assert((mqd_t)-1 != mqueue);
            assert(cur_attr != nullptr);
            std::cout << argv[i] << " mqueue has " << cur_attr->mq_curmsgs << " elements" << std::endl;

        }
        else if ( operation == "delete" ) {

            mqueue = mq_open(MQUEUE_NAME, O_RDONLY | O_NONBLOCK);
            if ( mqueue == -1 ) {
                std::cout << "mq_open() failed for " << argv[i] << " since it may not have been created" << std::endl;
                continue;
            }
            char buf[MAX_URL_LEN + 1];
            int sz;
            while ((sz = mq_receive(mqueue, buf, MAX_URL_LEN, nullptr)) != -1) {
                buf[sz] = '\0';
                std::cout << "removed from " << argv[i] << ":  " << buf << std::endl;
            }
        }


        // Detach the mqueue from this process
        mq_close(mqueue);
        if ( operation == "delete" ) {
            // Remove the mqueue from the system
            mq_unlink(MQUEUE_NAME);
            std::cout << argv[i] << " mqueue successfully deleted!" << std::endl;
        }
    }
}

int main(int argc, char **argv) {
    if ( argc < 2 || scb::String(argv[1]) == "-h" || scb::String(argv[1]) == "--help" ) {
        std::cout << "Usage: ./create-mq [mqueue-operation] [mqueue-name(s)]" << std::endl;
        std::cout << "Where [mqueue-operation] is exactly one of the following operations:" << std::endl;
        std::cout << "  create" << std::endl;
        std::cout << "  check" << std::endl;
        std::cout << "  delete" << std::endl;
        std::cout << "And where [mqueue-name(s)] can be one or more of the following queues seperated by a space:" << std::endl;
        std::cout << "  frontier" << std::endl;
        std::cout << "  newLinks" << std::endl;
        //TODO: Add more mqueues here as necessary, such as the index mqueue
        return 0;
    }

    scb::String operation = scb::String(argv[1]);
    if ( operation != "create" && operation != "check" && operation != "delete" ) {
        std::cout << "Invalid operation specified: " << argv[1] << ". Shutting down." << std::endl;
        return 0;
    }

    handleMqueue(argc, argv, operation);

}
