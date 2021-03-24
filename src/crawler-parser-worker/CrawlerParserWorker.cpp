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
#include "EnglishDictChecker.h"

int main( int argc, char **argv )
   {
   // according to SSL document, we should ignore this
   signal( SIGPIPE, SIG_IGN );

   // If we want to go with multiple queues with different priorities,
   //    add flag O_NONBLOCK and poll from them.
   mqd_t frontier = mq_open( FRONTIER_MQ, O_RDONLY );
   assert( frontier != -1 && "mq_open() failed" );

   mqd_t newLinks = mq_open( NEW_LINKS_QUEUE, O_WRONLY );
   assert( newLinks != -1 && "mq_open() failed" );

   mqd_t parsedDocs = mq_open( PARSED_DOCS_QUEUE, O_WRONLY );
   assert( parsedDocs != -1 && "mq_open() failed" );

   EnglishDictChecker englishDictChecker( "english.dict" );

   char buf[ MAX_URL_LEN + 1 ];
   while ( true )
      {
      ssize_t sz = mq_receive( frontier, buf, MAX_URL_LEN, nullptr );
      assert( sz != -1 && "mq_receive() failed" );
      buf[ sz ] = '\0';

      std::cout << "crawling: " << buf << std::endl;

      try
         {
         String url( buf );

         auto httpResp = FetchUrl( url );

         // filter out non-english page by HTTP
         if ( !httpResp.Language.empty() &&
              !httpResp.Language.hasPrefix( "en" ) )
            {
            std::cout << "not english by http" << std::endl;
            continue;
            }

         ParsedHtml parsedHtml( httpResp.Body );
         // filter out non-english page by HTML
         if ( !parsedHtml.Language.empty() &&
              !parsedHtml.Language.hasPrefix( "en" ) )
            {
            std::cout << "not english by http" << std::endl;
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
            std::cout << "not english by dict" << std::endl;
            continue;
            }

         // post parsedHtml to the indexer queue
         IndexerIngest ingest{ .Words = parsedHtml.Words,
                               .TitleWords = parsedHtml.TitleWords,
                               .Url = url };
         String blob = Serialize( ingest );
         // if too large a document, ignore
         if ( blob.size() <= PARSED_DOCS_QUEUE_MAX_MSGSIZE )
            mq_send( parsedDocs, blob.data(), blob.size(), 0 );

         // post URLs to the bloom filter queue
         ParsedUrl pageUrl( buf );
         for ( Link &link : parsedHtml.Links )
            {
            String normalizedUrl = pageUrl.Normalize( link.Href );
            std::cout << "posting: " << normalizedUrl << std::endl;
            mq_send( newLinks, normalizedUrl.data(), normalizedUrl.size(), 0 );
            }
         }
      catch ( std::exception &s )
         {
         // we're not going to recover anyway
         std::cout << "failed: " << s.what() << std::endl;
         continue;
         }
      }

   return 0;
   }
