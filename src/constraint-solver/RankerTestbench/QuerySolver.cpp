#include "../../query-parser/QueryParser.h"
#include "../../stl/HashBlob.h"
#include "../../stl/String.h"
#include "../ConstraintSolver.h"
#include "../ISR/AND.h"
#include "../ISR/Word.h"
#include "../QueryableChunk.h"
#include "Ranker.h"


int main( int argc, char **argv )
   {
   if ( argc < 3 )
      {
      fprintf( stderr,
               "Usage: %s [query] [index chunk 1] [index chunk 2] ...\n",
               argv[ 0 ] );
      return 1;
      }

   AST *queryAST = Parser()( argv[ 1 ] );

   // TODO: parallelize this

   //TODO: DELETE RANKER TESTBENCH
   Vector<QueryableChunk> queryableChunksToRanker;
   Vector<String> urlsToRanker;
   Vector<size_t> docIdsToRanker;
   AST * queryToRanker = queryAST;
   //TODO: DELETE RANKER TESTBENCH


   for ( int i = 2;  i < argc;  i++ )
      {
      QueryableChunk query( argv[ i ] );
      ISR *isr = query.MakeISR( queryAST );

      while ( !isr->IsEOF() )
         {
         auto url = query.Url( isr->DocID() );
         printf( "match post %d in doc %d ( %s )\n", isr->Get(), isr->DocID(),
                 url.cstr() );
         //TODO: DELETE RANKER TESTBENCH
         queryableChunksToRanker.pushBack(query);
         urlsToRanker.pushBack(url.cstr());
         docIdsToRanker.pushBack(isr->DocID());
         //TODO: DELETE RANKER TESTBENCH
         isr->NextDoc();
         }
      }
   
   dynamicRank(urlsToRanker, queryToRanker, queryableChunksToRanker, docIdsToRanker);

   return 0;
   }