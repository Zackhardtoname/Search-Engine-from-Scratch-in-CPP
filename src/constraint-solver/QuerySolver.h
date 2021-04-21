#ifndef QUERY_SOLVER
#define QUERY_SOLVER

#include "../query-parser/QueryParser.h"
//#include "../stl/HashBlob.h"
#include "../ranker/Ranker.h"
#include "../stl/String.h"
#include "ConstraintSolver.h"
#include "ISR/AND.h"
#include "ISR/Word.h"
#include "QueryableChunk.h"

// TODO return as reference
std::vector<displayField> handleQuery( scb::String query, int numChunks,
                                       char **chunks )
   {

   AST *queryAST = Parser()( query );

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

#endif // QUERY_SOLVER