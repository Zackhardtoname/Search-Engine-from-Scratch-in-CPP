//
// Created by youh on 4/13/21.
//

#ifndef SCOOBLE_SEARCH_WORD_H
#define SCOOBLE_SEARCH_WORD_H

#include "ISR.h"

class Word : public ISR
   {
      VLQStreamDecoder decoder;
      PostingListHeader postingListHeader;
      SyncTableEntry *syncTable;

      uint32_t curPost;
      int32_t docID;

      bool eof = false;

      uint8_t *firstPost;

   public:
      Word( const uint8_t *postingList, const ChunkHeader *chunkHeader )
          : ISR( chunkHeader )
         {
         postingListHeader =
            *reinterpret_cast<const PostingListHeader *>( postingList );
         syncTable =
            (SyncTableEntry *)( postingList + sizeof( postingListHeader ) );
         firstPost = reinterpret_cast<uint8_t *>(
            syncTable + postingListHeader.NumSyncTableEntries );
         decoder =
            VLQStreamDecoder( reinterpret_cast<const char *>( firstPost ) );

         curPost = decoder.Next();
         docID = 0;
         while ( curPost > chunk.LastPostTable[ docID ] )
            docID++;
         }

      void Next() override
         {
         if ( eof )
            return;

         // if calling Next() while we're already pointing to the last post
         if ( curPost == postingListHeader.LastPost )
            {
            eof = true;
            curPost = -1;
            return;
            }

         curPost += decoder.Next();

         // update docID if applicable
         // linear search because there are only a few thousands documents in
         // each chunks anyway
         // TODO: bsearch if run into performance issue
         while ( curPost > chunk.LastPostTable[ docID ] )
            docID++;
         }

      bool IsEOF() override
         {
         return eof;
         }

      uint32_t Get() override
         {
         return curPost;
         }

      void NextDoc() override
         {
         uint32_t curDocID = docID;
         while ( curPost <= chunk.LastPostTable[ curDocID ] && !IsEOF() )
            Next();
         }

      void Seek( uint32_t targetPost ) override
         {
         if ( targetPost < curPost )
            return;
         // if we are quite far away
         if ( targetPost - curPost > SyncTableEntry::NUM_POSTS_COVERED )
            {
            // TODO: bsearch if run into performance issue
            // find the first sync table entry > targetPost
            int upperbound = 0;
            while ( upperbound < postingListHeader.NumSyncTableEntries &&
                    syncTable[ upperbound ].Post <= targetPost )
               upperbound++;

            // no need to worry about 0, because the first entry in sync table
            // is always 0 so we jump to upperbound - 1
            auto &syncPoint = syncTable[ upperbound ];
            decoder = VLQStreamDecoder(
               reinterpret_cast<const char *>( firstPost + syncPoint.Offset ) );
            decoder
               .Next(); // the post pointed by sync point is also delta encoded
            curPost = syncPoint.Post;

            // update docID if applicable
            // TODO: bsearch if run into performance issue
            while ( curPost > chunk.LastPostTable[ docID ] )
               docID++;
            }

         // if we're nearby, just call Next() repeatedly
         while ( curPost <= targetPost && !IsEOF() )
            Next();
         }

      float Frequency() override
         {
         return static_cast<float>( postingListHeader.NumPosts ) /
                chunk.NumTotalPosts;
         }

      uint32_t DocID() override
         {
         return docID;
         }
   };

#endif // SCOOBLE_SEARCH_WORD_H
