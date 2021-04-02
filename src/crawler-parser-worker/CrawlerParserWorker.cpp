#include <csignal>
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

bool die = false;

void WorkerSIGTERMHandler( int signum )
   {
   die = true;
   std::cout << "dying....." << std::endl;
   }

int CrawlerParserWorkerMain( mqd_t frontier, mqd_t newLinks, mqd_t parsedDocs,
                             EnglishDictChecker &englishDictChecker )
   {

   struct sigaction sigactionTERM
      {
      };
   sigactionTERM.sa_handler = WorkerSIGTERMHandler;
   sigactionTERM.sa_flags =
      0; // no SA_RESTART so that wait() can be interrupted
   sigaction( SIGTERM, &sigactionTERM, nullptr );

   char buf[ MAX_URL_LEN + 1 ];
   for ( int numIndexedPages = 0;  !die;  )
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
         auto httpResp = FetchUrl( url );

         ParsedHtml parsedHtml( httpResp );
         // filter out non-english page by HTML
         if ( !parsedHtml.Language.empty() &&
              !parsedHtml.Language.hasPrefix( "en" ) )
            {
            std::cout << "not english by html: " << url << std::endl;
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
         if ( !englishDictChecker.Check( parsedHtml ) )
            {
            std::cout << "not english by dict: " << url << std::endl;
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
            std::cout << "posted to indexer queue, total " << ++numIndexedPages
                      << ": " << url << std::endl;
            }
         else
            std::cout << "doc too large: " << url << std::endl;

         // post URLs to the bloom filter queue
         ParsedUrl pageUrl( buf );
         for ( Link &link : parsedHtml.Links )
            {
            String normalizedUrl = pageUrl.Normalize( link.Href );
            mq_send( newLinks, normalizedUrl.data(), normalizedUrl.size(), 0 );
            }
         std::cout << "posted " << parsedHtml.Links.size()
                   << " to filter: " << url << std::endl;
         }
      catch ( std::exception &s )
         {
         // we're not going to recover anyway
         std::cout << "failed: " << s.what() << ": " << url << std::endl;
         continue;
         }
      }

   return 0;
   }
