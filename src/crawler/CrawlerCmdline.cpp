//
// Created by youh on 3/14/21.
//

#include <iostream>
#include <openssl/err.h>
#include <openssl/ssl.h>

#include "Crawler.h"
#include "ParsedUrl.h"

int main( int argc, char **argv )
   {
   if ( argc != 2 )
      {
      std::cerr << "Usage: ./crawler <URL>" << std::endl;
      return 1;
      }

   SSL_load_error_strings();
   ERR_load_crypto_strings();

   //    auto httpResp = FetchUrl("https://republic.co/gumroad");
   auto httpResp = FetchUrl( argv[ 1 ] );
   // auto httpResp = GetHtml("http://anglesharp.azurewebsites.net/Chunked"); //
   // test chunked
   std::cout << "---- body ----" << '\n'
             << httpResp;
   return 0;
   }