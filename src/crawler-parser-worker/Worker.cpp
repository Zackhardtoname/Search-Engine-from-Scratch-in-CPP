#include <mqueue.h>

#include "../stl/include/io/utils.h"
#include "../stl/include/RAII.h"

#include "../crawler/ParsedUrl.h"
#include "../crawler/Crawler.h"
#include "../crawler/ParsedHttpResponse.h"
#include "../parser/ParsedHtml.h"
#include "../defs.h"
#include "EnglishDictChecker.h"

int main(int argc, char **argv) {
    // If we want to go with multiple queues with different priorities,
    //    add flag O_NONBLOCK and poll from them.
    mqd_t frontier = mq_open(FRONTIER_MQ, O_RDONLY);
    assert(frontier != -1 && "mq_open() failed");

    EnglishDictChecker englishDictChecker("english.dict");

    char buf[MAX_URL_LEN + 1];
    while (true) {
        ssize_t sz = mq_receive(frontier, buf, MAX_URL_LEN, nullptr);
        assert(sz != -1 && "mq_receive() failed");
        buf[sz] = '\0';

        auto httpResp = FetchUrl(argv[2]);
        // filter out non-english page by HTTP
        if (!httpResp.Language.empty() && !httpResp.Language.hasPrefix("en"))
            continue;

        ParsedHtml parsedHtml(httpResp.Body);
        // filter out non-english page by HTML
        if (!parsedHtml.Language.empty() && !parsedHtml.Language.hasPrefix("en"))
            continue;

        // TODO: apply porter stemming here

        // check dictionary for language
        if (!englishDictChecker.Check(parsedHtml))
            continue;

        // TODO: post parsedHtml to the indexer queue

        // TODO: post URLs to the bloom filter queue
    }

    return 0;
}
