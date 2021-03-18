//
// Created by youh on 3/16/21.
//

#include <string>
#include <cstdint>
#include <cstring>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>

#include "../indexer/ContiguousChunk.h"

size_t fileSize( int f )
{
    struct stat fileInfo;
    fstat( f, &fileInfo);
    return fileInfo.st_size;
}

int main(int argc, char **argv) {
    if (argc != 2) {
        std::cout << "usage: " << argv[0] << " [chunk file]" << std::endl;
        return 1;
    }

    ContiguousChunk *chunk;

    // File read method
    // FILE *f = fopen(argv[1], "r");
    // char buf[10000];
    // size_t count = fread(buf, sizeof(ContiguousChunk), 1, f);

    // fclose(f);

    // std::cout << "Contiguous Chunk read: " << count << " element" << std::endl;
    
    // chunk =reinterpret_cast< ContiguousChunk* >(buf);
    // mmap() method

    int f = open(argv[1], O_RDONLY);
    if ( f == -1) {
        std::cerr << "Error opening file " << argv[1] << std::endl;
        return 1;
    }

    chunk = (ContiguousChunk*) mmap(nullptr, fileSize(f), PROT_READ, MAP_PRIVATE, f, 0 );
    if (chunk == MAP_FAILED) {
        std::cerr << "Error memory-mapping file " << argv[1] << std::endl;
        return 1;
    }



    std::cout << "Doc count: " << chunk->docCount << std::endl;
    std::cout << "Url List Offset: " << chunk->urlListOffset << std::endl;
    std::cout << "Dictionary Offset: " << chunk->dictionaryOffset << std::endl;
    std::cout << "Posting List Offset: " << chunk->postingListsOffset << std::endl;
}