//
// Created by youh on 3/2/21.
//

#ifndef SCOOBLE_SEARCH_CRAWLSIDEPIPELINE_H
#define SCOOBLE_SEARCH_CRAWLSIDEPIPELINE_H


#include "crawler/CrawlerParserManager.h"

class CrawlSidePipeline {
    CrawlerParserManager crawlerStage;
public:
    CrawlSidePipeline();

    void Run();
};


#endif //SCOOBLE_SEARCH_CRAWLSIDEPIPELINE_H
