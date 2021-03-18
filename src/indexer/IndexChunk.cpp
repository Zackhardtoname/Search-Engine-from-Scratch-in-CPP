//
// Created by youh on 3/14/21.
//

#include <cassert>
#include <ctime>

#include "IndexChunk.h"
#include "ContiguousChunk.h"

void IndexChunk::writeToDisk() {
    // Structure:
    // [Header] [URL List] [Dictionary (word -> posting list offset)] [Posting Lists]
    //
    // Header:
    // [i32 num of documents] [i32 offset for URL list] 
    // [i32 offset for posting list idx] [i32 offset for posting lists]
    //
}
