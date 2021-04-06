//
// Created by youh on 4/1/21.
//

#ifndef SCOOBLE_SEARCH_CRAWLERPARSERWORKER_H
#define SCOOBLE_SEARCH_CRAWLERPARSERWORKER_H

#include "EnglishDictChecker.h"
#include <atomic>
#include <mqueue.h>

extern EnglishDictChecker *englishDictChecker;
extern mqd_t frontier, newLinks, parsedDocs;
extern std::atomic<bool> die;

void *ThreadCrawlerParserWorker( void * );

#endif // SCOOBLE_SEARCH_CRAWLERPARSERWORKER_H
