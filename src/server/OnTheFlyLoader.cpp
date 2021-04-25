//
// Created by youh on 4/25/21.
//

#include "OnTheFlyLoader.h"
#include "../crawler/Crawler.h"
#include "../parser/ParsedHtml.h"
#include "../stl/RAII.h"

void *onTheFlyDownloader( void *p )
   {
   char *url = static_cast<char *>( p );
   Defer deferFreeUrl(
      [ & ]()
      {
         free( url );
      } );

   scb::String content;
   CURL *curl = GetCurlHandle( &content );
   Defer deferFreeCurl(
      [ & ]()
      {
         curl_easy_cleanup( curl );
      } );

   curl_easy_setopt( curl, CURLOPT_TIMEOUT, 5 ); // 5 second maximum
   curl_easy_setopt( curl, CURLOPT_URL, url );
   if ( curl_easy_perform( curl ) == CURLE_OK )
      {
         scb::String baseCommand = " ../snippet-generator/3_9_venv/bin/python3.9 -W ignore ../snippet-generator/snippet_generator.py ";
         scb::String errRedirect = " 2> /dev/null";
         scb::String command = baseCommand + url + errRedirect;
         FILE *pFile = popen(command.cstr(), "r");
         char buffer [100];

         if (pFile == NULL) perror ("Error opening file");
         else
         {
            while ( ! feof (pFile) )
            {
               if ( fgets (buffer , 100 , pFile) == NULL ) break;
            }
            fclose (pFile);
         }

         scb::String snip = buffer;
         return new OnTheFlyResources( url, content, snip );
      }
   else
      return nullptr;
   }

pthread_t SpawnFetchHtml( const scb::String &url )
   {
   pthread_t pt;
   pthread_create( &pt, nullptr, onTheFlyDownloader, strdup( url.cstr() ) );
   return pt;
   }
