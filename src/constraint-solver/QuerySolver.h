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
std::vector<displayField> handleQuery( scb::String queryIn, int numChunks,
                                       char **chunks )
   {

   AST *queryAST = Parser()( queryIn );

   // TODO: parallelize this

   std::vector<QueryableChunk> queryableChunksToRanker;
   std::vector<std::string> urlsToRanker;
   std::vector<size_t> docIdsToRanker;
   AST *queryToRanker = queryAST;

   for ( int i = 0;  i < numChunks;  i++ )
      {
      QueryableChunk query( chunks[ i ] );
      ISR *isr = query.MakeISR( queryAST );

      while ( !isr->IsEOF() )
         {
         auto url = query.Url( isr->DocID() );
         queryableChunksToRanker.push_back( query );
         std::string urlToAdd( url.cstr() );

         // Only add strings that match BEFORE the query string which starts
         // with a question mark (?) in order to remove duplicates with
         // erase and unique before returning
         size_t queryString = urlToAdd.find( '?' );
         if ( queryString == std::string::npos )
            urlsToRanker.push_back( urlToAdd );
         else
            urlsToRanker.push_back( urlToAdd.substr( 0, queryString ) );
         docIdsToRanker.push_back( isr->DocID() );
         isr->NextDoc();
         }
      }

   // Sort to prepare to remove duplicates
   std::sort( urlsToRanker.begin(), urlsToRanker.end(),
              []( const std::string &a, const std::string &b )
              {
                 return a < b;
              } );

   // Remove duplicates if the URLs match
   urlsToRanker.erase( std::unique( urlsToRanker.begin(), urlsToRanker.end() ),
                       urlsToRanker.end() );

   return dynamicRank( urlsToRanker, queryToRanker, queryableChunksToRanker,
                       docIdsToRanker );
   }

#endif // QUERY_SOLVER
