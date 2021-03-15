//
// Created by youh on 3/14/21.
//

#include "IndexChunk.h"

void IndexChunk::ingest(IndexerIngest &injest) {
    urls.pushBack(injest.url);
    for (String &word: injest.words)
        postingLists[word].pushBack(nextId++);
    for (String &word: injest.titleWords)
        postingLists[String("@") + word].pushBack(nextId++);
    documentBoundaries.pushBack(nextId);
}
