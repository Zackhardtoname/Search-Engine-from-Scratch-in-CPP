//
// Created by youh on 4/1/21.
//

#ifndef SCOOBLE_SEARCH_CRAWLERPARSERWORKER_H
#define SCOOBLE_SEARCH_CRAWLERPARSERWORKER_H

#include <mqueue.h>
#include "EnglishDictChecker.h"

int CrawlerParserWorkerMain(mqd_t frontier, mqd_t newLinks, mqd_t parsedDocs,
                            EnglishDictChecker &englishDictChecker );

#endif // SCOOBLE_SEARCH_CRAWLERPARSERWORKER_H
