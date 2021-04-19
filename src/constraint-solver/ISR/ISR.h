#ifndef ISR_H
#define ISR_H

/*
 * Index Stream Readers
 *
 * Interface for a general ISR and implementations for specific types of ISRs.
 *
 * ISRs are an abstraction used to search each index for matches.
 *
 * Created by mjwolf on 4/12/2021
 */

#include "../../indexer/ContiguousChunk.h"
#include "../../indexer/VLQ.h"

// A structure directly usable without complicated offset calculation.
struct ChunkRef
   {
      const LastPostTableEntry *LastPostTable;
      unsigned NumTotalPosts;
   };

// When an ISR is initialized, it should already point to the first occurence,
// or EOF.
class ISR
   {
public:
      ChunkRef chunk{};
protected:

      explicit ISR( const ChunkHeader *chunkHeader )
         {
         chunk.NumTotalPosts = chunkHeader->NumTotalPosts;
         chunk.LastPostTable = reinterpret_cast<const LastPostTableEntry *>(
            reinterpret_cast<const uint8_t *>( chunkHeader ) +
            chunkHeader->LastPostTableOffset );
         }

   public:
      // Advance this ISR by one position.
      virtual void Next() = 0;

      virtual bool IsEOF() = 0;

      virtual uint32_t Get() = 0;

      // Advance this ISR our of the current document.
      virtual void NextDoc() = 0;

      // Seek this ISR to at least the given position.
      virtual void Seek( uint32_t post ) = 0;

      virtual float Frequency() = 0;

      virtual uint32_t DocID() = 0;

      // First post of the doc this ISR is in.
      virtual uint32_t FirstPostOfCurDoc()
         {
         auto docID = DocID();
         if ( docID == 0 )
            return 0;
         else
            return chunk.LastPostTable[ docID - 1 ] + 1;
         }
   };

#endif // ISR_H
