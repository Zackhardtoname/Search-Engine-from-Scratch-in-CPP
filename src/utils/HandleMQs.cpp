//
// Created by epfeiffe on 3/18/21.
//

#include <mqueue.h>
#include <iostream>
#include <cassert>
#include <mqueue.h>
#include "../defs.h"
#include "../stl/String.h"

void createMqueue(int argc, char **argv) {
    for ( int i = 2; i < argc; ++i ) {

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
            return;
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

void checkMqueue(int argc, char **argv) {
    for ( int i = 2; i < argc; ++i ) {
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
            return;
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

void deleteMqueue(int argc, char **argv) {
    for ( int i = 2; i < argc; ++i ) {
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
            return;
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

    if ( scb::String(argv[1]) == "create" ) {
        createMqueue(argc, argv);
    }
    else if ( scb::String(argv[1]) == "create" ) {
        checkMqueue(argc, argv);
    }
    else if ( scb::String(argv[1]) == "create" ) {
        deleteMqueue(argc, argv);
    }
    else {
        std::cout << "Invalid operation specified: " << argv[1] << ". Shutting down." << std::endl;
    }
    return 0;
}
