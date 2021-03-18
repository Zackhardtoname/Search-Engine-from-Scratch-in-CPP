//
// Created by mjwolf on 3/17/21.
//

#include <cassert>
#include <ctime>

#include "ContiguousChunk.h"

int main(int argc, char **argv) {

    // generate filename from current time
    time_t timestamp = std::time(nullptr);
    tm *timeRepr = std::gmtime(&timestamp);
    char filename[1000];
    strftime(filename, 1000, "%Y_%m_%d_%H_%M_%S.chunk", timeRepr);

    // Open chunk file
    FILE *f = fopen(filename, "a"); // append
    long curSize = 0;

    // Create temporary chunk
    ContiguousChunk *chunk_ptr = new ContiguousChunk();
    chunk_ptr->docCount = 0;
    chunk_ptr->urlListOffset = 1;
    chunk_ptr->dictionaryOffset = 2;
    chunk_ptr->postingListsOffset = 3;
    char* chunkBuffer = reinterpret_cast<char*>(chunk_ptr);

    size_t written = fwrite(chunkBuffer, sizeof(ContiguousChunk), 1, f);
    
    std::cout << written << " elements written." << std::endl;

    fclose(f);
}