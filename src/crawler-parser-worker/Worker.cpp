#include <cassert>
#include <netdb.h>
#include <openssl/ssl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include "../stl/include/io/utils.h"
#include "../stl/include/RAII.h"
#include <iostream>
#include <utility>
#include <fstream>
#include <stdlib.h>

#include "../crawler/ParsedUrl.h"
#include "../crawler/Crawler.h"
#include "../crawler/ParsedHttpResponse.h"
#include "../parser/HtmlParser.h"

enum class Options
   {
   PrintWords,
   PrintTitleWords,
   PrintLinks,
   PrintBase
   };

Options option = Options::PrintWords;

void PrintParse( HtmlParser &parser )
   {
   switch ( option )
      {
      case Options::PrintWords:
         for ( auto w : parser.words )
            std::cout << w << std::endl;
         break;

      case Options::PrintTitleWords:
         for ( auto w : parser.titleWords )
            std::cout << w << std::endl;
         break;

      case Options::PrintLinks:
         for ( auto link : parser.links )
            {
            std::cout << link.URL << " ( ";
            for ( auto w : link.anchorText )
               std::cout << w << " ";
            std::cout << ")" << std::endl;
            }
         break;

      case Options::PrintBase:
         std::cout << parser.base << std::endl;
      }
   }


bool ValidOption( const char *o )
   {
   // Must be exactly one character.
   if ( !o[ 0 ] || o[ 1 ] )
      return false;

   switch ( o[ 0 ] )
      {
      case 'w':
         option = Options::PrintWords;
         break;
      case 't':
         option = Options::PrintTitleWords;
         break;
      case 'a':
         option = Options::PrintLinks;
         break;
      case 'b':
         option = Options::PrintBase;
         break;
      default:
         return false;
      }
   return true;
   }

int main(int argc, char **argv) {
    if ( !( ( argc == 3 && ValidOption( argv[ 1 ] ) ) ) )
    {
        std::cerr << "Usage: ./worker [wtab] <URL>\n";
        std::cerr << "Only one wtab option allowed at a time:\n";
        std::cerr << "  w  List the words, one per line.\n";
        std::cerr << "  t  List the title words, one per line.\n";
        std::cerr << "  a  list the links as URL( anchor text ), one per line.\n";
        std::cerr << "  b  Print any base that was found." << std::endl;

        return 1;
    }

    // TODO: Adjust endpoints to take a URL off of an mqueue and put a word index
    // structure on a different mqueue
    auto httpResp = FetchUrl(argv[2]);
    std::cout << httpResp.Body;

   std::cout << "---------- parser output --------\n";

   HtmlParser parser( httpResp.Body );
   PrintParse( parser );

   return 0;
}
