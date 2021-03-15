//
// Created by youh on 3/14/21.
//

#include "IndexChunk.h"

void IndexChunk::Ingest(IndexerIngest &injest) {
    for (String &word: injest.Words)
        postingLists[word].pushBack(nextId++);
    for (String &word: injest.TitleWords)
        postingLists[String("@") + word].pushBack(nextId++);
    documents.pushBack(
            {
                    .Url = injest.Url,
                    .Boundary = nextId
            });
    // TODO: update estimated size
}
