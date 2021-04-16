//
// Created by youh on 4/15/21.
//

#ifndef SCOOBLE_SEARCH_QUERYABLECHUNK_H
#define SCOOBLE_SEARCH_QUERYABLECHUNK_H

#include "../indexer/ContiguousChunk.h"
#include "../indexer/PorterStemming.h"
#include "../query-parser/QueryParser.h"
#include "../stl/utils.h"
#include "ISR/AND.h"
#include "ISR/OR.h"
#include "ISR/ISR.h"
#include "ISR/Null.h"
#include "ISR/Word.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fcntl.h>

// A representation of an index chunk that can be queried.
class QueryableChunk
   {
      scb::HashBlob *hashblob;
      ChunkHeader *chunk;

   public:
      explicit QueryableChunk( const char *chunkPath )
         {
         // Open chunk file for reading
         int fd = open( chunkPath, O_RDONLY );
         if ( fd == -1 )
            {
            fprintf( stderr, "Error opening file %s\n", chunkPath );
            exit( 1 );
            }

         // Map in chunk, initially cast as header
         chunk = reinterpret_cast<ChunkHeader *>(
            mmap( nullptr, FileSize( fd ), PROT_READ, MAP_PRIVATE, fd, 0 ) );

         if ( chunk == MAP_FAILED )
            {
            fprintf( stderr, "Error mmaping file %s\n", chunkPath );
            exit( 1 );
            }

         hashblob = reinterpret_cast<scb::HashBlob *>(
            reinterpret_cast<uint8_t *>( chunk ) +
            chunk->WordToPostingListDictionaryOffset );
         }

      ISR *MakeISR( AST *ast )
         {
         // if is a Binary Opnode
         if ( auto *bin = dynamic_cast<BinaryOpNode *>( ast ) )
            {

            ISR *leftISR = MakeISR( bin->left ),
               *rightISR = MakeISR( bin->right );
            scb::Vector<ISR *> clauses;

            switch ( bin->operation ) {
               case '&':

                  // if left/right is an AND, combine them together
                  if ( AND *leftAND = dynamic_cast<AND *>( leftISR ) )
                     for ( ISR *clause : leftAND->clauses )
                        clauses.pushBack( clause );
                  else
                     clauses.pushBack( leftISR );

                  if ( AND *rightAND = dynamic_cast<AND *>( rightISR ) )
                     for ( ISR *clause : rightAND->clauses )
                        clauses.pushBack( clause );
                  else
                     clauses.pushBack( rightISR );

                  return new AND( clauses, chunk );
                  
               case '|':

                  // if left/right is an OR, combine them together
                  if ( OR *leftOR = dynamic_cast<OR *>( leftISR ) )
                     for ( ISR *clause : leftOR->clauses )
                        clauses.pushBack( clause );
                  else
                     clauses.pushBack( leftISR );

                  if ( OR *rightOR = dynamic_cast<OR *>( rightISR ) )
                     for ( ISR *clause : rightOR->clauses )
                        clauses.pushBack( clause );
                  else
                     clauses.pushBack( rightISR );

                  return new OR( clauses, chunk );

               default:
                  assert( "operation not implemented" );
                  return nullptr;
            }

               

               

            }

         // If is a LiteralNode
         else if ( auto *lit = dynamic_cast<LiteralNode *>( ast ) )
            {
            auto stemmed = PorterStem( lit->word );
            const scb::SerialTuple *it = hashblob->Find( stemmed.cstr() );
            if ( it != nullptr )
               {
               auto k = it->GetKey();
               auto *postingList = reinterpret_cast<uint8_t *>( chunk ) +
                                   chunk->PostingListsOffset + it->Value;
               return new Word( postingList, chunk );
               }
            else
               {
               // no such word
               return new Null( chunk );
               }
            }
         else
            {
            assert( !"not implemented yet" );
            }
         return nullptr;
         }

      scb::String Url( uint32_t docID )
         {
         const char *urlList =
            reinterpret_cast<char *>( chunk ) + chunk->UrlListOffset;
         auto *urlOffsetTable = reinterpret_cast<UrlOffsetTableEntry *>(
            reinterpret_cast<char *>( chunk ) + chunk->UrlOffsetTableOffset );
         uint32_t offset = urlOffsetTable[ docID ];
         return scb::String( urlList + offset );
         }
   };

#endif // SCOOBLE_SEARCH_QUERYABLECHUNK_H
