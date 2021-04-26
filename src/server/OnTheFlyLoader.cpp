//
// Created by youh on 4/25/21.
//

#include "OnTheFlyLoader.h"
#include "../crawler/Crawler.h"
#include "../parser/ParsedHtml.h"
#include "../stl/RAII.h"

void *onTheFlyDownloader( void *p )
   {
   auto params = (onTheFlyDownloaderParams *)(p);
   char *url = params->url;
   char *query = params->query;

   Defer deferFreeUrl(
      [ & ]()
      {
         free( params );
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
         scb::String snip = "";

         if (params->useSnippet)
         {
            scb::String baseCommand = " ../snippet-generator/3_9_venv/bin/python3.9 -W ignore ../snippet-generator/snippet_generator.py ";
            scb::String errRedirect = " 2> /dev/null";
            scb::String command = baseCommand + url + " " + query + errRedirect;
            FILE *pFile = popen(command.cstr(), "r");
            char buffer [100];

            if (pFile == NULL) perror ("Error opening file");
            else
            {
               while ( ! feof (pFile) )
               {
                  if ( fgets (buffer , 100 , pFile) == NULL )
                     break;
                  else
                     snip += buffer;
               }
               fclose (pFile);
            }
         }

         return new OnTheFlyResources( url, content, snip );
      }
   else
      return nullptr;
   }

pthread_t SpawnFetchHtml(  void * params )
   {
   pthread_t pt;
   pthread_create( &pt, nullptr, onTheFlyDownloader, params );
   return pt;
   }
