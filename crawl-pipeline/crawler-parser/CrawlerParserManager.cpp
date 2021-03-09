//
// Created by youh on 3/2/21.
//

#include "CrawlerParserManager.h"
#include "../defs.h"
// TODO: don't include like this

#include <unistd.h>

#include "CrawlerParser.h"

CrawlerParserManager::CrawlerParserManager() {
    urlMQ = mq_open(URL_MQ, O_RDONLY);
}

void CrawlerParserManager::SpawnOne() {
    if (fork() == 0) {
        CrawlerParser(urlMQ).RunForever();
    }
}

