#pragma once
#ifndef RANKER
#define RANKER

#include "../../indexer/PorterStemming.h"
#include "../../query-parser/QueryParser.h"
#include "../../stl/String.h"
#include "../ISR/ISR.h"
#include "../QueryableChunk.h"
#include "aho-corasick.h"
#include <atomic>
#include <bits/stdc++.h>
#include <math.h>
#include <pthread.h>
#define toInt( c ) (int)( ( (int)c ) - (int)97 )
// "harry potter and the goblet of fire"--initial test
// harry * potter *
// best link types:
// .gov -> .edu -> .com -> .net -> .others

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

// Dynamic Rank Factors
/*
    Consider ANDs and ORs as the same thing
    URLs - is the query in the URL?
        - URL is the only ranking factor where we worry about NOTs
        - Set rank to half of whatever it should if a not word is found
    Content - is the query in sequence in the document?
        - Consider ANDs
        - (cats*dogs) + (pigs*sheep) + birds (turn AST into prefix tree)
    Content - how many times is the query in sequence in the document?
    Content - is the query in the title?
    Content - how far apart are the query words in the document? (if not in
   sequence?) Content - individual word counts (the more repetitions the better)
    Content - does it have more of the rare words that were in the query?

*/

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
/*
urls need to be stemmed
    leg->legs
    just pass into pre-written function

now we have a littany of words
    //the longer the matched query string, the more it is weighted
Now we make the trie

*/
// a b c
// an be co
// an b co
// a||an b||be c||co

// how to toot your own horn
// to toot your own horn

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

// concatenate a cat
// pthread_mutex_t lock;
void rankerSearcher( sharedISRStruct arg )
   {
   struct sharedISRStruct *currStruct = &arg;
   auto *isr = currStruct->isr;
   auto query = currStruct->query;
   auto edge = currStruct->edge;
   auto urlNum = currStruct->urlNum;
   while ( (long long)isr->Get() < edge )
      {
      // std::cout<<isr->Get()<<" must be less than " << edge<<"\n";
      shared_isr_map[ urlNum ][ isr->Get() ] = query;
      isr->Next();
      }
   // pthread_mutex_unlock(&lock);

   // isr.Get();
   // gets a specific ISR, and the specific word it is pertaining to.
   // looks for instances of the word

   // if instance is found
   // get location of instance from ISR, call it idx
   // lock the shared structure
   // if the current word is longer than the one stored, replace it
   // lengths provided in intOrder from strMap.
   // else dont
   // unlock
   // keep searching.
   }

pthread_mutex_t lock;
std::vector<std::vector<std::string>> splitUrls;
std::vector<std::vector<QueryableChunk>> splitChunks;
std::vector<std::vector<size_t>> splitID;
std::vector<std::string> globalStemmed;

void *mainLoop( void *arg )
   {
   pthread_mutex_lock( &lock );
   int currInt = *(int *)arg;
   // struct mainLoopStruct * currStruct =   ((struct mainLoopStruct *)(arg));
   auto urlStart = currInt * 1000;
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
      // std::cout<<"\tThread "<<" reached query loop "<<i<<" of
      // "<<urls.size()<<"."<<std::endl;
      for ( auto query : stemmed )
         {
         // std::cout<<"\t\tThread is at "<<query<<" word."<<std::endl;
         // pthread_mutex_lock(&lock);
         AST *ast = new LiteralNode( String( query.c_str() ) );
         // pthread_mutex_unlock(&lock);
         auto isr = currChunk.MakeISR( ast );
         // we want the isr to be on the location of the first matching query
         // Starting at the beginning of the document specified.
         auto edge = isr->chunk.LastPostTable[ currID ];

         if ( !currID )
            isr->Seek( isr->chunk.LastPostTable[ 0 ] );
         else
            isr->Seek( isr->chunk.LastPostTable[ currID - 1 ] );
         // pthread_mutex_unlock(&lock);
         // std::cout<<"\tThread "<<" reached end of first atomic
         // section."<<std::endl;

         // hasnt fallen off the edge
         if ( isr->Get() <= edge )
            {
            // pthread_mutex_lock(&lock);
            struct sharedISRStruct args =
               sharedISRStruct{ isr, strMap[ query ], edge, i };
            rankerSearcher( args );
            // pthread_mutex_unlock(&lock);
            }
         // std::cout<<"\tThread "<<" reached end of loop."<<std::endl;
         }
      }
   // std::cout<<"\tThread "<<" reached end of mainLoop."<<std::endl;
   // pthread_exit(0);
   // free(arg);

   pthread_mutex_unlock( &lock );
   return NULL;
   }

void dynamicRank( std::vector<std::string> urls, AST *Query,
                  std::vector<QueryableChunk> chunks,
                  std::vector<size_t> docID )
   {
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

   // Content Ranking
   // if query is in the document
   // whole query
   // number of consecutive matches
   // consecutive meaning allowed to alternate between steemed and vanilla
   // e.g. ("cat and dog" would find perfect match with "cats and dogs")
   // Scoring:
   // Number of words in a row consecutively add more
   // Given query: "I want to die"
   // If the webpage contains "I want to die"
   // The score is:
   // \Sigma_{k=1}^{query.size()}{Occ[k] * e^{-(query.size()-k)}}
   // Also counting number of times words show up.
   // create a mapping of words, and increment counter when they show up
   // after sorting all queries, at the end, multiply by length

   // CONTENT RANKING
   // needs to be highly multithreaded.
   // first implement naively, then go back and multithread.

   // ISR for every word
   // all ISRs search till end at same time
   // insert into shared structure location, idx of what they found
   // queries[0]->0 etc. allows for super quick search, number of occurences,
   // and consecutive terms all at the same time. In o(n)

   // this for loop is actually a shit ton of threads, and this isnt saved in
   // this function all done simultaneously
   std::vector<double> contentRank( urls.size() );

   std::vector<float> savedExponent;

   // std::cout<<"FUCK FUCK FUCK FUCK\n";

   for ( int i = 1;  i <= stemmed.size();  ++i )
      {
      savedExponent.push_back( ( queries.size() - i ) *
                               ( queries.size() - i ) );
      }
   shared_isr_map.resize( urls.size() );

   size_t numUrlThreads = ceil( urls.size() / 1000.0 );
   if ( numUrlThreads <= 1 )
      {
      for ( int i = 0;  i < urls.size();  ++i )
         {
         // std::cout<<"Iteration: "<<i<<"\n";
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
                  sharedISRStruct{ isr, strMap[ query ], edge, i };
               // std::cout<<"Before Function\n";
               rankerSearcher( args );
               // std::cout<<"After Function\n";
               ++idx;
               // pthread_mutex_unlock(&lock);
               }
            }
         // for (int j = 0; j<vectorOfThreads.size(); ++j){
         //        pthread_join(vectorOfThreads[j], NULL);
         //}
         // vectorOfThreads.clear();
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
         // std::cout<<"\n--------------------Loop "<< i
         // <<"--------------------"<<std::endl;

         pthread_t urlThread;
         args[ i ] = i;
         // mainLoopStruct{currUrl, stemmed, splitUrls[i], splitChunks[i],
         // splitID[i]};
         mainThreadVector[ i ] = urlThread;
         // std::cout<<"Thread "<<i<<" launched.\n";
         pthread_create( &mainThreadVector[ i ], NULL, mainLoop, &args[ i ] );
         // mainLoop(&args);
         // std::cout<<"Main Loop Ended"<<std::endl;
         currUrl += splitUrls[ i ].size();
         // pthread_detach(mainThreadVector[i]);
         // std::cout<<"i is currently: " << i << std::endl;
         }
      // std::cout<<stemmed[0]<<"\n";
      pthread_mutex_unlock( &lock );

      for ( int i = 0;  i < numUrlThreads;  ++i )
         {
         // std::cout<<"Try to detach"<<std::endl;
         pthread_join( mainThreadVector[ i ], NULL );
         }
      }
   std::cout << "Finished Ranking.\n";
   // exit(0);
   // TODO FIX ACTUAL RANKING
   for ( int i = 0;  i < shared_isr_map.size();  ++i )
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
   std::cout << "Finished Dynamic Scoring.\n";
   staticRank( urls );
   // std::cout<<"Finished Static Scoring.\n";

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

   std::vector<std::pair<double, std::string>> bigPairNRG;
   for ( int i = 0;  i < urls.size();  ++i )
      {
      bigPairNRG.push_back( { contentRank[ i ], urls[ i ] } );
      }
   std::sort( bigPairNRG.begin(), bigPairNRG.end(),
              []( const std::pair<int, std::string> &a,
                  const std::pair<int, std::string> &b )
              {
                 return a.first < b.first;
              } );

   for ( int i = 0;  i < bigPairNRG.size();  ++i )
      {
      std::cout << bigPairNRG[ i ].second << ":\t" << bigPairNRG[ i ].first
                << "\n";
      }

   // found dynamic url rank.
   // i want to learn computer science

   /*
   for(std::stringurl : urls) {

       std::std::stringfindWords(url.cstr());

       // Start at begin + 7 to avoid the https:// or http://
       //auto first = find_if(findWords.begin() + 7, findWords.end(),
   ::isalnum);
       //auto last = find_if_not(first, findWords.end(), ::isalnum);
       //while (first != last) {
       //    std::std::stringnewWord(first, last);
       //    urlWords.push_back(newWord);
       //    first = find_if(last, findWords.end(), ::isalnum);
       //    last = find_if_not(first, findWords.end(), ::isalnum);
       //}

       //for(auto word : urlWords) {
       //    std::cout << word << '\t';
       //}
       //std::cout<<"\n";

       urlWords.clear();
   }
   */
   }
#endif
/*
    Ranker:
        What is the input?
            Links
            Query
        Where is the input?
        How are we recieving it?
        When are we recieving it?
        What do we want to return?
            Rank
                Rank factors + Final Rank
            Url
            Title - ask Michael/han how to get this?
            ~snippet~ - optional
    What we dont want to worry about:
*/
