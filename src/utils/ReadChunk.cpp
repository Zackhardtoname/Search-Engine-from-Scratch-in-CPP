//
// Created by youh on 3/16/21.
//

#include <cstdint>
#include <cstring>
#include <fcntl.h>
#include <string>
#include <sys/mman.h>
#include <sys/stat.h>

#include "../indexer/ContiguousChunk.h"
#include "../indexer/VLQ.h"

using std::cerr;
using std::cout;

size_t fileSize( int f )
   {
   struct stat fileInfo;
   fstat( f, &fileInfo );
   return fileInfo.st_size;
   }

int main( int argc, char **argv )
   {
   if ( argc != 2 )
      {
      std::cout << "usage: " << argv[ 0 ] << " [chunk file]" << std::endl;
      return 1;
      }

   ContiguousChunk *chunk;

   // File read method
   // FILE *f = fopen(argv[1], "r");
   // char buf[10000];
   // size_t count = fread(buf, sizeof(ContiguousChunk), 1, f);

   // fclose(f);

   // std::cout << "Contiguous Chunk read: " << count << " element" <<
   // std::endl;

   // chunk =reinterpret_cast< ContiguousChunk* >(buf);
   // mmap() method

   int f = open( argv[ 1 ], O_RDONLY );
   if ( f == -1 )
      {
      cerr << "Error opening file " << argv[ 1 ] << std::endl;
      return 1;
      }

   chunk = (ContiguousChunk *)mmap( nullptr, fileSize( f ), PROT_READ,
                                    MAP_PRIVATE, f, 0 );
   if ( chunk == MAP_FAILED )
      {
      cerr << "Error memory-mapping file " << argv[ 1 ] << std::endl;
      return 1;
      }

   // Read header

   cout << "Chunk Header:" << std::endl;
   cout << "\tDocument count: " << chunk->header.docCount << std::endl;
   cout << "\tDictionary Offset: " << chunk->header.dictionaryOffset
        << std::endl;
   cout << "\tPosting List Offset: " << chunk->header.postingListsOffset
        << std::endl;
   cout << "\tEnd Offset: " << chunk->header.endOffset << std::endl;

   // Read URL List
   cout << "URLs:" << std::endl;
   const char *cursor = chunk->data;

   // TODO check if faster to just print character by character
   while ( cursor < chunk->data + chunk->header.dictionaryOffset )
      {
      cout << "\t" << cursor << std::endl;
      cursor += strlen( cursor ) + 1;
      }

   // TODO print dictionary
   cursor = chunk->data + chunk->header.postingListsOffset;
   // Print decoded posts
   VLQStreamDecoder decoder( cursor );
   cout << "Posting Lists:" << std::endl;
   int plCounter = 0;
   uint64_t decoded;

   // For each posting list
   while ( cursor < chunk->data + chunk->header.endOffset )
      {

      const PostingListHeader *plHeader =
         reinterpret_cast<const PostingListHeader *>( cursor );

      cursor += sizeof( PostingListHeader ) + plHeader->postsOffset;

      decoder.setCursor( cursor );

      cout << "\t" << plCounter++ << " (postsOffset = " << plHeader->postsOffset
           << "):" << std::endl;
      while ( ( decoded = decoder.next() ) != 0 )
         {
         cout << "\t\t" << decoded << std::endl;
         }

      cursor = decoder.getCursor();
      }
   }