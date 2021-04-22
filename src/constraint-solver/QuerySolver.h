#ifndef SCOOBLE_SEARCH_QUERY_SOLVER_H
#define SCOOBLE_SEARCH_QUERY_SOLVER_H

#include "../query-parser/QueryParser.h"
//#include "../stl/HashBlob.h"
#include "../ranker/Ranker.h"
#include "../stl/String.h"
#include "ConstraintSolver.h"
#include "ISR/AND.h"
#include "ISR/Word.h"
#include "QueryableChunk.h"

// TODO return as reference
std::vector<displayField> handleQuery( const scb::String &query, int numChunks,
                                       char **chunks )
   {

   AST *queryAST = ParseQuery( query );

   // TODO: parallelize this

   // TODO: DELETE RANKER TESTBENCH
   std::vector<QueryableChunk> queryableChunksToRanker;
   std::vector<std::string> urlsToRanker;
   std::vector<size_t> docIdsToRanker;
   AST *queryToRanker = queryAST;
   // TODO: DELETE RANKER TESTBENCH

   for ( int i = 0;  i < numChunks;  i++ )
      {
      QueryableChunk query( chunks[ i ] );
      ISR *isr = query.MakeISR( queryAST );

      while ( !isr->IsEOF() )
         {
         auto url = query.Url( isr->DocID() );
         // printf( "match post %d in doc %d ( %s )\n", isr->Get(),
         // isr->DocID(),
         //        url.cstr() );
         // TODO: DELETE RANKER TESTBENCH
         queryableChunksToRanker.push_back( query );
         urlsToRanker.push_back( std::string( url.cstr() ) );
         docIdsToRanker.push_back( isr->DocID() );
         // TODO: DELETE RANKER TESTBENCH
         isr->NextDoc();
         }
      }

   return dynamicRank( urlsToRanker, queryToRanker, queryableChunksToRanker,
                       docIdsToRanker );
   }

#endif // SCOOBLE_SEARCH_QUERY_SOLVER_H