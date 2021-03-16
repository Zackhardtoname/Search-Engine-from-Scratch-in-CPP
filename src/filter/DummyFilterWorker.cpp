//
// Created by youh on 3/16/21.
//

#include <cassert>
#include <netdb.h>
#include <openssl/ssl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <set>
#include <string>
#include "../stl/include/io/utils.h"
#include "../stl/include/RAII.h"
#include <iostream>
#include <utility>
#include <fstream>
#include <cstdlib>
#include <mqueue.h>

#include "../crawler/ParsedUrl.h"
#include "../crawler/Crawler.h"
#include "../crawler/ParsedHttpResponse.h"
#include "../parser/ParsedHtml.h"
#include "../defs.h"

std::set<std::string> seen;

int main() {
    mq_attr frontierMQAttr{};
    frontierMQAttr.mq_flags = 0;
    frontierMQAttr.mq_maxmsg = 256;
    frontierMQAttr.mq_msgsize = MAX_URL_LEN;
    frontierMQAttr.mq_curmsgs = 0;
    mqd_t frontier = mq_open(FRONTIER_MQ, O_WRONLY | O_CREAT, 0644, &frontierMQAttr);
    assert(frontier != -1 && "mq_open() failed");

    const char *seed = "https://news.ycombinator.com/";
    mq_send(frontier, seed, strlen(seed), 0);

    mq_attr newLinksMQAttr{};
    newLinksMQAttr.mq_flags = 0;
    newLinksMQAttr.mq_maxmsg = 256;
    newLinksMQAttr.mq_msgsize = MAX_URL_LEN;
    newLinksMQAttr.mq_curmsgs = 0;
    mqd_t newLinks = mq_open(NEW_LINKS_QUEUE, O_RDONLY | O_CREAT, 0644, &newLinksMQAttr);

    char buf[MAX_URL_LEN + 1];
    while (true) {
        ssize_t sz = mq_receive(newLinks, buf, MAX_URL_LEN, nullptr);
        assert(sz != -1 && "mq_receive() failed");
        buf[sz] = '\0';

        std::string url(buf);
        if (seen.contains(url)) {
            std::cout << "seen: " << url << std::endl;
            continue;
        }

        seen.insert(url);
        mq_send(frontier, url.data(), url.size(), 0);
        std::cout << "posted: " << url << std::endl;
    }
    assert(newLinks != -1 && "mq_open() failed");
}