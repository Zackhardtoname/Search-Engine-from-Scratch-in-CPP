//
// Created by youh on 3/16/21.
//

#include <mqueue.h>
#include <iostream>
#include <cassert>
#include "../defs.h"

int main() {
    mq_attr attr{};
    mqd_t frontier = mq_open(FRONTIER_MQ, O_RDONLY);
    assert(frontier != -1 && "mq_open() failed");
    mq_getattr(frontier, &attr);

    printf("frontier: %d/%d\n", attr.mq_curmsgs, attr.mq_maxmsg);

    mqd_t newLinks = mq_open(NEW_LINKS_QUEUE, O_RDONLY);
    assert(frontier != -1 && "mq_open() failed");
    mq_getattr(newLinks, &attr);
    printf("new links: %d/%d\n", attr.mq_curmsgs, attr.mq_maxmsg);
}