//
// Created by youh on 3/16/21.
//

#include <cstdint>
#include <cstring>
#include <fcntl.h>
#include <string>
#include <sys/mman.h>
#include <sys/stat.h>

#include "../../indexer/ContiguousChunk.h"
#include "../../indexer/VLQ.h"
#include "../../stl/HashBlob.h"

using std::cerr;
using std::cout;
using std::endl;
using scb::HashBlob;

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
      std::cout << "usage: " << argv[ 0 ] << " [chunk file]" << endl;
      return 1;
      }

   ChunkHeader *header;

   int f = open( argv[ 1 ], O_RDONLY );
   if ( f == -1 )
      {
      cerr << "Error opening file " << argv[ 1 ] << endl;
      return 1;
      }

   // Map in chunk, initially cast as header
   header = (ChunkHeader *)mmap( nullptr, fileSize( f ), PROT_READ,
                                    MAP_PRIVATE, f, 0 );
   if ( header == MAP_FAILED )
      {
      cerr << "Error memory-mapping file " << argv[ 1 ] << endl;
      return 1;
      }

   // * ===== Read header =====
   cout << "Chunk Header:" << endl;
   cout << "\tDocument count: " << header->DocumentCount << endl;
   cout << "\tDocument Table Start: " << header->DocumentTableOffset << endl;
   cout << "\tDictionary Offset: " << header->WordToPostingListDictionaryOffset
        << endl;
   cout << "\tPosting List Offset: " << header->PostingListsOffset
        << endl;


   sizeof ( DocumentTableEntry );

   // * ===== Read Document Table =====
   const DocumentTableEntry *currentEntry = reinterpret_cast<DocumentTableEntry*>(header + 1);

   cout << "Document Table:" << endl;
   cout << "\tIndex\tUrlOffs" << endl; 
   // For each Document Table Entry
   for ( size_t i = 0 ; i < header->DocumentCount; ++i, ++currentEntry ) {
      cout << "\t[" << i << "]\t" << currentEntry->UrlOffset << endl;
   }

   // * ===== Read URL List =====
   const char *currentURL = reinterpret_cast<const char*>(currentEntry);

   //std::cout<<"The thing tested against is: " << reinterpret_cast<const char*> (header) + header->PostingListsOffset<<"\n";
   //exit(0);
   cout << "URLs:" << endl;

   // Print each URL
   for(size_t i = 0; i < header->DocumentCount ; ++i ) {
      cout << "\t[" << i << "]\t" << currentURL << endl;
      currentURL += strlen( currentURL ) + 1;
   }

   // * ===== Read Dictionary HashBlob =====
   const auto blob = reinterpret_cast<const HashBlob*>(currentURL);

   cout << "Dictionary HashBlob:" << endl;
   
   // Print blob header and contents
   cout << *blob << endl;
   

   // * ===== Read Posting Lists =====
   auto PlIt = blob->begin();
   ++PlIt;
   size_t ctr = 0;
   
   // For each posting list
   for ( auto PlIt = blob->begin(); PlIt != blob->end() ; ++PlIt) {
      const char *cursor = reinterpret_cast<const char*>(header) + header->PostingListsOffset + PlIt->Value;
      
      // cout << "Posting List [" << ctr++ << "] for '" << PlIt->Key << "':" << endl;
      cout << "Posting List [" << ctr++ << "] for '" << PlIt->GetKey() << "':" << endl;

      // Read and print posting list header
      const PostingListHeader *plHeader =
         reinterpret_cast<const PostingListHeader *>( cursor );

      cout << "\tHeader:" << std::endl;
      cout << "\t\tNumber of posts: " << plHeader->NumPosts 
      << "\n\t\tNumber of Sync Table Entries: " << plHeader->NumSyncTableEntries << endl;

      // Read and print Sync Table
      cout << "\tSync Table:\tOffs:\tPost:" << std::endl;
      const SyncTableEntry *syncEntry = reinterpret_cast<const SyncTableEntry*>(plHeader+1);
      for ( size_t i = 0; i < plHeader->NumSyncTableEntries; ++i) {
         cout << "\t\t[" << i << "]\t" << syncEntry->Offset << "\t" << syncEntry->Post << std::endl;
         ++syncEntry;
      }

      size_t postsOffset = static_cast<uint32_t>( sizeof( PostingListHeader ) + plHeader->NumSyncTableEntries * sizeof( SyncTableEntry ) );

      cursor += postsOffset;

      // Read and print posts
      cout << "\tPosts:\tDelta:\tAbsolute" << std::endl;

      uint64_t decoded;
      uint64_t prevAbsolute = 0;
      VLQStreamDecoder decoder( cursor );
      // decoder.setCursor( cursor );
      
      while ( ( decoded = decoder.Next() ) != 0 )
         {
         cout << "\t\t" << decoded << "\t" << prevAbsolute + decoded << endl;
         prevAbsolute += decoded;
         }

      cursor = decoder.getCursor();

      }

   cout << "End of chunk." << endl;
   }
