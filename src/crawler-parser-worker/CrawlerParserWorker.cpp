#include <csignal>
#include <curl/curl.h>
#include <mqueue.h>

#include "../stl/RAII.h"
#include "../stl/utils.h"

#include "../crawler/Crawler.h"
#include "../crawler/ParsedHttpResponse.h"
#include "../crawler/ParsedUrl.h"
#include "../defs.h"
#include "../indexer/PorterStemming.h"
#include "../parser/ParsedHtml.h"
#include "CrawlerParserWorker.h"
#include "EnglishDictChecker.h"

EnglishDictChecker *englishDictChecker;
mqd_t frontier, newLinks, parsedDocs;
std::atomic<bool> die = false;
std::atomic<int> numSentToIndexer = 0;

void *ThreadCrawlerParserWorker( void * )
   {
   String content;
   CURL *curl = GetCurlHandle( &content );

   char buf[ MAX_URL_LEN + 1 ];
   while ( !die.load() )
      {

      ssize_t sz = mq_receive( frontier, buf, MAX_URL_LEN, nullptr );
      if ( sz == -1 && errno == EINTR )
         {
         die = true;
         break;
         }
      assert( sz != -1 && "mq_receive() failed" );
      buf[ sz ] = '\0';
      String url( buf );

      try
         {
         curl_easy_setopt( curl, CURLOPT_URL, url.cstr() );
         content.clear();
         CURLcode code = curl_easy_perform( curl );
         HandleCurlCode( curl, code );

         ParsedHtml parsedHtml( content );
         // filter out non-english page by HTML
         if ( !parsedHtml.Language.empty() &&
              !parsedHtml.Language.hasPrefix( "en" ) )
            {
#ifdef VERY_VERBOSE
            std::cout << "not english by html: " << url << std::endl;
#endif
            continue;
            }

         // apply porter stemming here, since we are going to apply
         // the transformation eventually anyway, we might as well reuse
         // some of that work.
         for ( String &word : parsedHtml.Words )
            word = PorterStem( word );
         for ( String &word : parsedHtml.TitleWords )
            word = PorterStem( word );

         // check dictionary for language
         if ( !englishDictChecker->Check( parsedHtml ) )
            {
#ifdef VERY_VERBOSE
            std::cout << "not english by dict: " << url << std::endl;
#endif
            continue;
            }

         // post parsedHtml to the indexer queue
         IndexerIngest ingest{ .Words = parsedHtml.Words,
                               .TitleWords = parsedHtml.TitleWords,
                               .Url = url };
         String blob = Serialize( ingest );
         // if too large a document, ignore
         if ( blob.size() <= PARSED_DOCS_QUEUE_MAX_MSGSIZE )
            {
            if ( mq_send( parsedDocs, blob.data(), blob.size(), 0 ) != 0 )
               std::cout << "mq_send() error: " << strerror( errno ) << " : "
                         << url << std::endl;
#ifdef VERY_VERBOSE
            std::cout << "posted to indexer queue, total " << ++numSentToIndexer
                      << ": " << url << std::endl;
#else
            if ( ( ++numSentToIndexer ) % 2000 == 0 )
               std::cout << "posted to indexer queue, total " << std::endl;
#endif
            }
         else
            {
#ifdef VERY_VERBOSE
            std::cout << "doc too large: " << url << std::endl;
#endif
            }

         // post URLs to the bloom filter queue
         ParsedUrl pageUrl( buf );
         for ( String &link : parsedHtml.Links )
            {
            String normalizedUrl = pageUrl.Normalize( link );
            mq_send( newLinks, normalizedUrl.data(), normalizedUrl.size(), 0 );
            }
#ifdef VERY_VERBOSE
         std::cout << "posted " << parsedHtml.Links.size()
                   << " to filter: " << url << std::endl;
#endif
         }
      catch ( std::exception &s )
         {
         // we're not going to recover anyway
         std::cout << "failed: " << s.what() << ": " << url << std::endl;
         continue;
         }
      }

   return nullptr;
   }
