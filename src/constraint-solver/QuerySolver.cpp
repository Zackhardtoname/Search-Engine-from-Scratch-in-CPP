#include "../stl/HashBlob.h"
#include "../stl/String.h"
#include "ConstraintSolver.h"
#include "ISR/AND.h"
#include "ISR/Word.h"
#include "QueryListener.h"

const char *path =
   "/home/youh/eecs440/scooble_search/bin/2021_04_14_15_42_47_fnkxhd.chunk";

int main( int argc, char **argv )
   {
   // Open chunk file for reading
   int fd = open( path, O_RDONLY );
   if ( fd == -1 )
      {
      cerr << "Error opening file " << path << endl;
      exit( 1 );
      }

   // Map in chunk, initially cast as header
   auto *chunk = reinterpret_cast<ChunkHeader *>(
      mmap( nullptr, fileSize( fd ), PROT_READ, MAP_PRIVATE, fd, 0 ) );
   if ( chunk == MAP_FAILED )
      {
      cerr << "Error mmaping file " << path << endl;
      exit( 1 );
      }

   auto *blob = reinterpret_cast<scb::HashBlob *>(
      reinterpret_cast<uint8_t *>( chunk ) +
      chunk->WordToPostingListDictionaryOffset );

   const scb::SerialTuple *rstA = blob->Find( "a" );
   auto *postingListA = reinterpret_cast<uint8_t *>( chunk ) +
                        chunk->PostingListsOffset + rstA->Value;

   const scb::SerialTuple *rstB = blob->Find( "b" );
   auto *postingListB = reinterpret_cast<uint8_t *>( chunk ) +
                        chunk->PostingListsOffset + rstB->Value;

   Word isrA( postingListA, chunk );
   Word isrB( postingListB, chunk );

   AND isrAND( { &isrA, &isrB }, chunk );

   /*
   while (!isrA.IsEOF()) {
      std::cout << "A post: " << isrA.Get() << " in doc " << isrA.DocID() <<
   std::endl; isrA.Next();
   }
    */

   while ( !isrAND.IsEOF() )
      {
      std::cout << "A&B post: " << isrAND.Get() << " in doc " << isrAND.DocID()
                << std::endl;
      isrAND.NextDoc();
      }

   return 0;
   }