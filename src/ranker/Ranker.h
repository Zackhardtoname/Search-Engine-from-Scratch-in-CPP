#ifndef RANKER
#define RANKER

#include "../constraint-solver/ISR/ISR.h"
#include "../constraint-solver/QueryableChunk.h"
#include "../indexer/PorterStemming.h"
#include "../query-parser/QueryParser.h"
#include "../stl/String.h"
#include "aho-corasick.h"
#include <atomic>
#include <bits/stdc++.h>
#include <math.h>
#include <pthread.h>

#define threadThreshold 800

#define toInt( c ) (int)( ( (int)c ) - (int)97 )

struct displayField
   {
      scb::String title;
      scb::String URL;
      double rank;

      const bool operator==( const displayField &check ) const
         {
         return check.URL == URL;
         }
   };

template <typename T>
std::vector<std::vector<T>> splitVector( const std::vector<T> &vec, size_t n )
   {
   std::vector<std::vector<T>> ret;
   for ( size_t i = 0;  i < vec.size();  i += n )
      {
      auto last = vec.size() > i + n ? i + n : vec.size();
      ret.emplace_back( vec.begin() + i, vec.begin() + last );
      }
   return ret;
   }

std::unordered_map<std::string, float> staticRankedLinks;

struct LinkInfo
   {
      std::string link;
      // Static ranking variables

      bool https;
      int domainType;
      size_t length;
      size_t numSlashes;

      // Final score as a result of static and dynamic ranking
      double score;
   };

auto staticRankComparator( const LinkInfo &linkA, const LinkInfo &linkB )
   -> bool
   {
   if ( linkA.https == linkB.https )
      {
      if ( linkA.domainType == linkB.domainType )
         {
         if ( linkA.length == linkB.length )
            {
            if ( linkA.numSlashes == linkB.numSlashes )
               {
               return linkA.link < linkB.link;
               }
            return linkA.numSlashes < linkB.numSlashes;
            }
         return linkA.length < linkB.length;
         }
      return linkA.domainType > linkB.domainType;
      }
   return linkA.https > linkB.https;
   }

// TODO: Run the static rank once at the very beginning and memory map the rank
// values to a file We would compute the dynamic rank at runtime
void staticRank( std::vector<std::string> urls )
   {
   size_t numLinks = urls.size();
   double linkFactor = 1.0 / numLinks;
   std::vector<LinkInfo> linkInfo( numLinks );
   uint idx = 0;
   size_t numSlashes;
   int domainType;
   bool https;

   for ( auto link : urls )
      {
      if ( link == "https://saader.tumblr.com/tagged/JIMMY-FALLON" )
         {
         numSlashes = 0;
         }
      numSlashes = 0;
      https = false;
      auto start = link.end();

      while ( *start != '.' )
         {
         if ( *start == '/' )
            ++numSlashes;
         --start;
         }
      ++start;

      if ( link[ 4 ] == 's' )
         https = true;

      switch ( *start )
         {
         case 'g':
            domainType = 5;
            break;
         case 'e':
            domainType = 4;
            break;
         case 'c':
            domainType = 3;
            break;
         case 'n':
            domainType = 2;
            break;
         default:
            domainType = 1;
            break;
         }
      --start;
      --start;
      if ( *start != '\0' && *( start-- ) == 'r' )
         if ( *start != '\0' && *( start-- ) == 'l' )
            if ( *start != '\0' && *( start-- ) == 'b' )
               if ( *start != '\0' && *( start-- ) == 'm' )
                  domainType -= 10000;
      linkInfo[ idx++ ] = { link, https, domainType, link.size(), numSlashes };
      }

   // std::sort(linkInfo.begin(), linkInfo.end(), staticRankComparator);

   for ( idx = 0;  idx < linkInfo.size();  ++idx )
      staticRankedLinks[ linkInfo[ idx ].link ] +=
         linkInfo[ idx ].domainType / ( linkFactor );
   // return linkInfo;
   }

void nodeReturner(
   AST *Query,
   std::vector<std::string> &queries /*, std::vector<std::string> &disallow*/,
   bool sign )
   {
   if ( auto *literal = dynamic_cast<LiteralNode *>( Query ) )
      {
      if ( sign )
         queries.push_back( std::string( literal->word.cstr() ) );
      // else
      //    disallow.push_back(literal->word);
      }
   else if ( auto *binary = dynamic_cast<BinaryOpNode *>( Query ) )
      {
      nodeReturner( binary->left, queries /*, disallow*/, sign );
      if ( binary->operation == '-' )
         nodeReturner( binary->right, queries, /*disallow,*/ !sign );
      else
         nodeReturner( binary->right, queries, /*disallow,*/ sign );
      }
   else if ( auto *unary = dynamic_cast<UnaryOpNode *>( Query ) )
      {
      if ( unary->operation == '-' )
         nodeReturner( unary->param, queries /*, disallow*/, !sign );
      else
         nodeReturner( unary->param, queries, /*disallow,*/ sign );
      }
   }

std::vector<std::map<uint32_t, int>> shared_isr_map;
std::unordered_map<std::string, int> strMap;
std::unordered_map<int, int> intOrder;

struct sharedISRStruct
   {
      ISR *isr;
      int query;
      long long edge;
      size_t urlNum;
   };

struct mainLoopStruct
   {
      size_t currURLStart;
      std::vector<std::string> query;
      std::vector<std::string> splitUrls;
      std::vector<QueryableChunk> splitChunks;
      std::vector<size_t> splitID;
   };

pthread_mutex_t lock;
void rankerSearcher( sharedISRStruct arg )
   {
   struct sharedISRStruct *currStruct = &arg;
   auto *isr = currStruct->isr;
   auto query = currStruct->query;
   auto edge = currStruct->edge;
   auto urlNum = currStruct->urlNum;
   while ( (long long)isr->Get() < edge )
      {
      pthread_mutex_lock( &lock );
      shared_isr_map[ urlNum ][ isr->Get() ] = query;
      pthread_mutex_unlock( &lock );
      isr->Next();
      }
   }

std::vector<std::vector<std::string>> splitUrls;
std::vector<std::vector<QueryableChunk>> splitChunks;
std::vector<std::vector<size_t>> splitID;
std::vector<std::string> globalStemmed;
std::vector<double> contentRank;

void *mainLoop( void *arg )
   {
   // pthread_mutex_lock(&lock);
   int currInt = *(int *)arg;
   // struct mainLoopStruct * currStruct =   ((struct mainLoopStruct *)(arg));
   auto urlStart = currInt * threadThreshold;
   std::vector<std::string> stemmed = globalStemmed;
   auto urls = splitUrls[ currInt ];
   std::vector<QueryableChunk> chunks = splitChunks[ currInt ];
   auto docID = splitID[ currInt ];

   // std::cout<<"\tThread "<<" reached for loop of size
   // "<<urls.size()<<"."<<std::endl;
   for ( int i = urlStart;  i < urlStart + urls.size();  ++i )
      {
      auto currUrl = urls[ i - urlStart ];
      // pthread_mutex_lock(&lock);
      auto currChunk = chunks[ i - urlStart ];
      // pthread_mutex_unlock(&lock);
      auto currID = docID[ i - urlStart ];

      // create an isr for each word, corresponding to the current document
      for ( auto query : stemmed )
         {
         AST *ast = new LiteralNode( String( query.c_str() ) );
         auto isr = currChunk.MakeISR( ast );
         // we want the isr to be on the location of the first matching query
         // Starting at the beginning of the document specified.
         auto edge = isr->chunk.LastPostTable[ currID ];

         if ( !currID )
            isr->Seek( isr->chunk.LastPostTable[ 0 ] );
         else
            isr->Seek( isr->chunk.LastPostTable[ currID - 1 ] );

         // hasnt fallen off the edge
         if ( isr->Get() <= edge )
            {
            struct sharedISRStruct args =
               sharedISRStruct{ isr, strMap[ query ], edge, (size_t)i };
            rankerSearcher( args );
            }
         }
      }

   for ( int i = urlStart;  i < urlStart + urls.size();  ++i )
      {
      uint32_t prevKey = shared_isr_map[ i ].begin()->first;
      int prevVal = shared_isr_map[ i ].begin()->second;
      uint currLen = 1;

      for ( auto occurence : shared_isr_map[ i ] )
         {
         // order
         if ( occurence.first - prevKey == 1 )
            {
            if ( occurence.second - prevVal == 1 )
               {
               ++currLen;
               contentRank[ i ] += contentRank[ i ] / 20.0 +
                                   currLen * ( intOrder[ occurence.second ] *
                                               intOrder[ occurence.second ] );
               }
            }
         else
            {
            currLen = 1;
            }
         // length of the string, TODO figure out weighting
         contentRank[ i ] += intOrder[ occurence.second ] - 2 > 0
                                ? intOrder[ occurence.second ] - 2
                                : 0;
         }
      }
   return NULL;
   }

std::vector<displayField> dynamicRank( std::vector<std::string> urls,
                                       AST *Query,
                                       std::vector<QueryableChunk> chunks,
                                       std::vector<size_t> docID )
   {
   std::ios_base::sync_with_stdio( 0 );
   Trie trie;
   std::vector<std::string> queries;
   shared_isr_map.resize( urls.size() );
   // convert the AST into a vector of words in 'queries', and all NOTs into
   // 'disallow'
   nodeReturner( Query, queries, 1 );

   // do porter stemming on all of the query words.
   // defined in indexer (../indexer/PorterStemming.h)
   uint qsz = queries.size();
   std::vector<std::string> stemmed( queries.size() );
   for ( int i = 0;  i < qsz;  ++i )
      {
      stemmed[ i ] =
         std::string( PorterStem( String( queries[ i ].c_str() ) ).cstr() );
      strMap[ stemmed[ i ] ] = i;
      intOrder[ i ] = stemmed[ i ].size();
      }

   contentRank.resize( ( urls.size() ) );

   std::vector<float> savedExponent;

   shared_isr_map.resize( urls.size() );

   size_t numUrlThreads = ceil( urls.size() / threadThreshold );
   if ( numUrlThreads <= 1 )
      {
      for ( int i = 0;  i < urls.size();  ++i )
         {
         auto currUrl = urls[ i ];
         auto currChunk = chunks[ i ];
         auto currID = docID[ i ];
         // create an isr for each word, corresponding to the current document
         // std::vector<pthread_t> vectorOfThreads;
         uint idx = 0;
         for ( auto query : stemmed )
            {
            // std::cout<<"Query: "<<query<<"\n";
            AST *ast = new LiteralNode( String( query.c_str() ) );
            auto isr = currChunk.MakeISR( ast );
            // we want the isr to be on the location of the first matching query
            // Starting at the beginning of the document specified.

            auto edge = isr->chunk.LastPostTable[ currID ];

            if ( !currID )
               isr->Seek( isr->chunk.LastPostTable[ 0 ] );
            else
               isr->Seek( isr->chunk.LastPostTable[ currID - 1 ] );

            // hasnt fallen off the edge
            if ( isr->Get() <= edge )
               {
               // pthread_t new_thread;
               struct sharedISRStruct args =
                  sharedISRStruct{ isr, strMap[ query ], edge, (size_t)i };
               rankerSearcher( args );
               ++idx;
               }
            }
         }
      }

   else
      {
      // each thread must have a certain amount of information
      // each thread must have its specified urls
      // each thread must have its specified chunks
      // each thread must have its specified docIDs
      // each thread must have the overall query.
      splitUrls = splitVector( urls, numUrlThreads );
      splitChunks = splitVector( chunks, numUrlThreads );
      splitID = splitVector( docID, numUrlThreads );

      globalStemmed = stemmed;
      size_t currUrl = 0;
      pthread_t mainThreadVector[ numUrlThreads ];
      pthread_mutex_lock( &lock );
      int args[ numUrlThreads ];
      for ( size_t i = 0;  i < numUrlThreads;  ++i )
         {
         pthread_t urlThread;
         args[ i ] = i;
         mainThreadVector[ i ] = urlThread;
         pthread_create( &mainThreadVector[ i ], NULL, mainLoop, &args[ i ] );
         currUrl += splitUrls[ i ].size();
         }
      pthread_mutex_unlock( &lock );

      for ( int i = 0;  i < numUrlThreads;  ++i )
         {
         pthread_join( mainThreadVector[ i ], NULL );
         }
      }
   std::cout << "Finished Ranking.\n";
   // TODO FIX ACTUAL RANKING
   std::cout << "Finished Dynamic Scoring.\n";
   staticRank( urls );

   // URL RANKING
   for ( auto stem : stemmed )
      queries.push_back( stem );
   std::sort( queries.begin(), queries.end() );

   // Implement Aho-Corsick to reduce time to search URLs to linear with O(m +
   // q)*O(n) with preprocessing rather than O(nlogn)*O(*q*d*n)
   std::set<int> found;
   std::vector<bool> _found( queries.size() );
   std::vector<double> urlRank( urls.size() );

   for ( int i = 0;  i < queries.size();  ++i )
      trie.addPattern( queries[ i ], i );

   uint urlNUM = 0;

   for ( auto url : urls )
      {
      trie.processText( url, found );
      for ( auto it = found.begin();  it != found.end();  it++ )
         {
         _found[ *it ] = 1;
         }
      for ( int i = 0;  i < _found.size();  ++i )
         urlRank[ urlNUM ] += i * _found[ i ], _found[ i ] = 0;

      contentRank[ urlNUM ] +=
         staticRankedLinks[ url ] * .7 + urlRank[ urlNUM ] / 42.0;
      ++urlNUM;
      }
   std::cout << "Finished URL Scoring.\n";

   std::vector<displayField> rankedResults( urls.size() );
   for ( int i = 0;  i < urls.size();  ++i )
      {
      // TODO: figure out the title and set it here
      rankedResults[ i ].title = urls[ i ].c_str();
      rankedResults[ i ].URL = urls[ i ].c_str();
      rankedResults[ i ].rank = contentRank[ i ];
      }

   std::sort( rankedResults.begin(), rankedResults.end(),
              []( const displayField &a, const displayField &b )
              {
                 return a.rank > b.rank;
              } );
   // Remove duplicates if the URLs match
   rankedResults.erase(
      std::unique( rankedResults.begin(), rankedResults.end() ),
      rankedResults.end() );

   return rankedResults;
   }
#endif // RANKER