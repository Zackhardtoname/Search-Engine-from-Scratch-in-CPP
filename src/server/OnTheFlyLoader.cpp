//
// Created by youh on 4/25/21.
//

#include "OnTheFlyLoader.h"
#include "../crawler/Crawler.h"
#include "../stl/RAII.h"
#include "../parser/ParsedHtml.h"

void *onTheFlyDownloader(void *p) {
   char *url = static_cast<char *>(p);
   Defer deferFreeUrl([&]() {
      free(url);
   });

   scb::String content;
   CURL *curl = GetCurlHandle(&content);
   Defer deferFreeCurl([&]() {
      curl_easy_cleanup(curl);
   });

   curl_easy_setopt(curl, CURLOPT_TIMEOUT, 5); // 5 second maximum
   curl_easy_setopt(curl, CURLOPT_URL, url);
   if (curl_easy_perform(curl) == CURLE_OK) {
      return new OnTheFlyResources(url, content);
   } else
      return nullptr;

}

pthread_t SpawnFetchHtml(const scb::String &url) {
   pthread_t pt;
   pthread_create(&pt, nullptr, onTheFlyDownloader, strdup(url.cstr()));
   return pt;
}
