//
// Created by youh on 3/16/21.
//

#include <cassert>
#include <netdb.h>
#include <openssl/ssl.h>
#include <sys/socket.h>
#include <poll.h>
#include <sys/types.h>
#include <unistd.h>
#include <set>
#include <string>
#include "../stl/utils.h"
#include "../stl/RAII.h"
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
#include "../stl/String.h"
#include "../stl/Vector.h"

using scb::String;
using scb::Vector;

std::set<std::string> seen;
Vector<String> frontierCache;

int main() {
    // open the queues
    mq_attr frontierMQAttr{};
    frontierMQAttr.mq_flags = 0;
    frontierMQAttr.mq_maxmsg = 256;
    frontierMQAttr.mq_msgsize = MAX_URL_LEN;
    frontierMQAttr.mq_curmsgs = 0;
    mqd_t frontier = mq_open(FRONTIER_MQ, O_WRONLY | O_NONBLOCK | O_CREAT, 0644, &frontierMQAttr);
    assert(frontier != -1 && "mq_open() failed");

    const char *seed = "https://news.ycombinator.com/";
    mq_send(frontier, seed, strlen(seed), 0);

    mq_attr newLinksMQAttr{};
    newLinksMQAttr.mq_flags = 0;
    newLinksMQAttr.mq_maxmsg = 256;
    newLinksMQAttr.mq_msgsize = MAX_URL_LEN;
    newLinksMQAttr.mq_curmsgs = 0;
    mqd_t newLinks = mq_open(NEW_LINKS_QUEUE, O_RDONLY | O_NONBLOCK | O_CREAT, 0644, &newLinksMQAttr);

    // handle queue events
    pollfd events[2] = {
            {.fd = frontier, .events = POLLOUT,},
            {.fd = newLinks, .events = POLLIN,}
    };
    char buf[MAX_URL_LEN + 1];
    while (true) {
        poll(events, 2, -1);

        // if frontier queue writable
        bool blocked = false;
        while (events[0].revents && !frontierCache.empty() && !blocked) {
            String url = frontierCache.back();
            frontierCache.popBack();
            if (url.size() > MAX_URL_LEN)
                continue;

            if (mq_send(frontier, url.data(), url.size(), 0) == -1) {
                if (errno == EAGAIN)
                    blocked = true;
                else
                    assert(false && "mq_send() failed");
            } else {
                std::cout << "posted: " << url << std::endl;
                sleep(1); // TODO: delete this after we have crawl rate control
            }
        }

        // if new links queue readble
        while (events[1].revents) {
            ssize_t sz = mq_receive(newLinks, buf, MAX_URL_LEN, nullptr);
            if (sz == -1) {
                if (errno == EAGAIN)
                    break; // blocked
                else
                    assert(false && "mq_receive() failed");
            }
            buf[sz] = '\0';

            std::string url(buf);
            if (seen.contains(url)) {
                std::cout << "seen: " << url << std::endl;
                continue;
            }

            // not seen before
            seen.insert(url);
            frontierCache.emplaceBack(url.c_str());
        }
    }
    assert(newLinks != -1 && "mq_open() failed");
}