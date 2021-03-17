#include <cassert>
#include <netdb.h>
#include <openssl/ssl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include "../stl/utils.h"
#include "../stl/RAII.h"
#include <iostream>
#include <utility>
#include <fstream>
#include <stdlib.h>
#include <mqueue.h>
#include <cassert>


#include "../crawler/ParsedUrl.h"
#include "../crawler/Crawler.h"
#include "../crawler/ParsedHttpResponse.h"
#include "../parser/ParsedHtml.h"
#include "../defs.h"

enum class Options
   {
   PrintWords,
   PrintTitleWords,
   PrintLinks,
   PrintBase
   };

Options option = Options::PrintWords;

void PrintParse( ParsedHtml &parsedHtml )
   {
   switch ( option )
      {
      case Options::PrintWords:
         for ( auto w : parsedHtml.Words )
            std::cout << w << std::endl;
         break;

      case Options::PrintTitleWords:
         for ( auto w : parsedHtml.TitleWords )
            std::cout << w << std::endl;
         break;

      case Options::PrintLinks:
         for ( auto link : parsedHtml.Links )
            {
            std::cout << link.Href << " ( ";
            for ( auto w : link.AnchorText )
               std::cout << w << " ";
            std::cout << ")" << std::endl;
            }
         break;

      case Options::PrintBase:
         std::cout << parsedHtml.Base << std::endl;
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
    if ( !( ( argc > 2 && ValidOption( argv[ 1 ] ) ) ) )
    {
        std::cerr << "Usage: ./worker [wtab] <URL>\n";
        std::cerr << "Only one wtab option allowed at a time:\n";
        std::cerr << "  w  List the words, one per line.\n";
        std::cerr << "  t  List the title words, one per line.\n";
        std::cerr << "  a  list the links as URL( anchor text ), one per line.\n";
        std::cerr << "  b  Print any base that was found." << std::endl;
        std::cerr << "Run with: ./worker [wtab] <URL> clear\n";
        std::cerr << "To clear the message queue. Any value can be used instead of clear, just as long as there is a fourth argument\n";

        return 1;
    }

    // TODO: Adjust endpoints to take a URL off of an mqueue and put a word index
    // structure on a different mqueue

    mqd_t mq;
    struct mq_attr attr;
    char buffer[MAX_URL_LEN + 1];
    char answer = 'Y';

    // initialize the queue attributes
    // TODO: Figure out how to set attr.mq_msgmsg to a higher value
    attr.mq_flags = 0;
    attr.mq_maxmsg = 256;
    attr.mq_msgsize = MAX_URL_LEN;
    attr.mq_curmsgs = 0;

    // create the message queue
    mq = mq_open(FRONTIER_MQ, O_CREAT | O_RDWR, 0644, &attr);
    assert((mqd_t)-1 != mq);

    // Clear the message queue and reset it
    if(argc > 3) {
        assert((mqd_t)-1 != mq_close(mq));
        assert((mqd_t)-1 != mq_unlink(FRONTIER_MQ));
        std::cout << "Ran cleanup for mqueue. Rerun worker now.\n";
        return 0;
    }
        
    // Add the passed in URL as the first link on the message queue
    // TODO: Have this read off a seperate message queue that the parent process
    // initializes with a seed list of URLs
    assert(0 <= mq_send(mq, argv[2], MAX_URL_LEN, 0));

    do {
    
        ssize_t bytes_read;

        // receive first URL on the mq
        bytes_read = mq_receive(mq, buffer, MAX_URL_LEN, NULL);
        assert(bytes_read >= 0);
        buffer[bytes_read] = '\0';
        std::cout << "url read from MQ: " << buffer << std::endl;

        // Crawl and Parse the URL popped from the message queue
        auto httpResp = FetchUrl(buffer);
        ParsedHtml parsedHtml(httpResp.Body );
        //PrintParse( parser );

        // After parsing the original URL, send every URL within its HTML
        // to the message queue
        for(auto link : parsedHtml.Links) {
            // First check if the message queue is full, and break if it is
            struct mq_attr *cur_attr = &attr;
            mq_getattr(mq, cur_attr);
            assert((mqd_t)-1 != mq);
            assert(cur_attr != nullptr);
            if(cur_attr->mq_curmsgs == attr.mq_maxmsg) {
                std::cout << "Message queue has reached the max capacity of " << attr.mq_maxmsg << std::endl;
                break;
            }

            // Error handling since the parser doesn't format each link correctly
            // TODO: Handle more cases here and consider moving this logic to the Parser
            if(link.Href.substr(0, 4) != "http") {
                // If missing the http characters, this is likely a subdomain of the current
                // page we are on
                link.Href = String(buffer) + link.Href;
            }
            int space_position = link.Href.find(" ");
            while(space_position != String::npos) {
                // If there are spaces in the URL, replace with the space string "%20" for
                // the GET requests to render correctly in the Crawler code
                link.Href = link.Href.replace( space_position, 1, "%20" );
                space_position = link.Href.find(" ");
            }

            // Add the reformatted URL to the message queue
            // TODO: Use the parser variable to format the parsed data into and index and
            // add the index structure to the index message queue
            // TODO: Add the URL to a seperate message queue that will be handled with the
            // bloom filter/robots.txt process to check if the URL should be placed back
            // on the frontier message queue (the mq that this process reads from)
            assert(0 <= mq_send(mq, link.Href.cstr(), MAX_URL_LEN, 0));
            std::cout << "url added: " << link.Href << std::endl;
        }

        // Prompt user to parse the next URL on the message queue
        std::cout << "Parse another link? (Y/N): ";
        std::cin >> answer;

    } while (answer == 'Y' || answer == 'y');
   
    std::cout << "Parsing stopped, shutting down the worker.\n";
    // cleanup
    assert((mqd_t)-1 != mq_close(mq));
    assert((mqd_t)-1 != mq_unlink(FRONTIER_MQ));

   return 0;
}
