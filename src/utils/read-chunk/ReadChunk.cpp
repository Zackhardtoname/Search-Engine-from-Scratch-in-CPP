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


void blobHeaderPrint(const HashBlob *curr){
   std::cout<<"Blob Header:\n\tMagic Number: "<<curr->MagicNumber<<
      "\n\tVersion: "<<curr->Version<<"\n\tBlobSize: "<<
      "\n\tBlobSize: "<<curr->BlobSize<<"\n\tNumber of Buckets: "<<
      curr->NumberOfBuckets<<"\n";
   
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


   // * ===== Read header =====
   const DocumentTableEntry *currentEntry = reinterpret_cast<DocumentTableEntry*>(header + 1);

   cout << "Document Table:" << endl;
   cout << "\tIndex\tUrlOffs\tLastPost" << endl; 
   // For each Document Table Entry
   for ( size_t i = 0 ; i < header->DocumentCount; ++i ) {
      cout << "\t" << i << ":\t" << currentEntry->UrlOffset << "\t" << currentEntry->LastPost << endl;
   }

   // * ===== Read URL List =====
   const char *currentURL = reinterpret_cast<const char*>(currentEntry + 1);

   std::cout<<"URL is: "<<currentURL<<"\n";
   //std::cout<<"The thing tested against is: " << reinterpret_cast<const char*> (header) + header->PostingListsOffset<<"\n";
   //exit(0);
   cout << "URLs:" << endl;

   // Print each URL
   for(size_t i = 0; i < header->DocumentCount ; ++i ) {
      cout << "\t" << currentURL << endl;
      currentURL += strlen( currentURL ) + 1;
   }

   // ===== Read Dictionary HashBlob =====
   const auto blob = reinterpret_cast<const HashBlob*>(currentURL);

   std::cout << "Dictionary HashBlob:" << endl;
   
   // Print blob header
   blobHeaderPrint(blob);
   blob->Print(cout);
   
   
   // Print Contents
   
   
   
   
   
   /*
   cursor = chunk->data + chunk->header.PostingListsOffset;
   // Print decoded posts
   VLQStreamDecoder decoder( cursor );
   cout << "Posting Lists:" << endl;
   int plCounter = 0;
   uint64_t decoded;

   // For each posting list
   while ( cursor < chunk->data + chunk->header.EndOffset )
      {

      const PostingListHeader *plHeader =
         reinterpret_cast<const PostingListHeader *>( cursor );

      cursor += sizeof( PostingListHeader ) + plHeader->postsOffset;

      decoder.setCursor( cursor );

      cout << "\t" << plCounter++ << " (postsOffset = " << plHeader->postsOffset
           << "):" << endl;
      while ( ( decoded = decoder.Next() ) != 0 )
         {
         cout << "\t\t" << decoded << endl;
         }

      cursor = decoder.getCursor();
      }
   */
   }