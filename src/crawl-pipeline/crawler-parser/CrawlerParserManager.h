//
// Created by youh on 3/2/21.
//

#ifndef SCOOBLE_SEARCH_CRAWLERPARSERMANAGER_H
#define SCOOBLE_SEARCH_CRAWLERPARSERMANAGER_H

#include <mqueue.h>


// The crawler stage that takes in URLs, crawl them and post them to the controller
class CrawlerParserManager {
    mqd_t urlMQ;

public:
    CrawlerParserManager();

    void SpawnOne();
};


#endif //SCOOBLE_SEARCH_CRAWLERPARSERMANAGER_H
