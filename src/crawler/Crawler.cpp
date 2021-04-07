#include "Crawler.h"
#include "../defs.h"
#include "../stl/RAII.h"
#include "../stl/UniquePointer.h"
#include "../stl/utils.h"
#include "ParsedHttpResponse.h"
#include "ParsedUrl.h"
#include <cassert>
#include <curl/curl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

size_t curlWriteCallback( char *ptr, size_t size, size_t nmemb, void *pString )
   {
   auto *content = static_cast<String *>( pString );
   for ( int i = 0;  i < size * nmemb;  i++ )
      content->pushBack( ptr[ i ] );
   return size * nmemb;
   }

CURL *GetCurlHandle( String *content )
   {
   CURL *curl = curl_easy_init();

   // setup writing to `content`
   curl_easy_setopt( curl, CURLOPT_NOPROGRESS, 1 ); // no progress bar
   curl_easy_setopt( curl, CURLOPT_WRITEFUNCTION, curlWriteCallback );
   curl_easy_setopt( curl, CURLOPT_WRITEDATA, content );

   // setup options
   curl_easy_setopt( curl, CURLOPT_FAILONERROR, 1 );    // error on >400
   curl_easy_setopt( curl, CURLOPT_FOLLOWLOCATION, 1 ); // redirection
   curl_easy_setopt( curl, CURLOPT_MAXREDIRS, 10 ); // 10 redirections at most
   curl_easy_setopt(
      curl, CURLOPT_USERAGENT,
      "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, "
      "like Gecko) Chrome/88.0.4324.190 Safari/537.36 zacklt@umich.edu" );
   curl_easy_setopt( curl, CURLOPT_HTTPGET, 1 );          // use GET
   curl_easy_setopt( curl, CURLOPT_MAXFILESIZE, 5 * MB ); // maximum size 5 MB
   curl_easy_setopt( curl, CURLOPT_TIMEOUT, 30 );         // 30 second maximum
   curl_easy_setopt( curl, CURLOPT_LOW_SPEED_LIMIT,
                     10 * 1024 ); // 10 KB/s is slow
   curl_easy_setopt( curl, CURLOPT_LOW_SPEED_TIME,
                     10 ); // abort if 10 seconds too slow
   curl_easy_setopt( curl, CURLOPT_CONNECTTIMEOUT,
                     10 ); // abort if 10 seconds not connect

   return curl;
   }

void HandleCurlCode( CURL *curl, CURLcode code )
   {
   switch ( code )
      {
      case CURLE_OK:
         break;
      case CURLE_HTTP_RETURNED_ERROR:
         {
         long respCode = -1;
         curl_easy_getinfo( curl, CURLINFO_HTTP_CODE, &respCode );
         throw std::runtime_error( "HTTP " + std::to_string( respCode ) );
         }
      default:
         throw std::runtime_error( curl_easy_strerror( code ) );
      }
   }
