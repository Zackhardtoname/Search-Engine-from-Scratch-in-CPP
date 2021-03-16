//
// Created by youh on 3/16/21.
//

#include <mqueue.h>
#include <iostream>
#include <cassert>
#include "../defs.h"

int main() {
    mqd_t frontier = mq_open(FRONTIER_MQ, O_RDONLY | O_NONBLOCK);
    assert(frontier != -1 && "mq_open() failed");

    mqd_t newLinks = mq_open(NEW_LINKS_QUEUE, O_RDONLY | O_NONBLOCK);
    assert(frontier != -1 && "mq_open() failed");

    char buf[MAX_URL_LEN + 1];
    int sz;
    while ((sz = mq_receive(frontier, buf, MAX_URL_LEN, nullptr)) != -1) {
        buf[sz] = '\0';
        std::cout << "frontier: " << buf << std::endl;
    }

    while ((sz = mq_receive(newLinks, buf, MAX_URL_LEN, nullptr)) != -1) {
        buf[sz] = '\0';
        std::cout << "new links: " << buf << std::endl;
    }

    mq_close(frontier);
    mq_close(newLinks);
    mq_unlink(FRONTIER_MQ);
    mq_unlink(NEW_LINKS_QUEUE);
}