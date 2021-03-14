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

#include "ParsedUrl.h"
#include "Worker.h"
#include "ParsedHttpResponse.h"
#include "HtmlParser.h"

// TODO: make it so that we abort on lengthy response, and/or timeout
// TODO: setup build file for test
// TODO: error handling, catch the exception thrown


// ----------------------------------- CRAWLER CODE -----------------------------------

String HttpGet(const ParsedUrl &url) {
    // Create and connect a socket.
    addrinfo *addr = QueryHost(url.Host, strlen(url.Port) == 0 ? url.Service : url.Port);
    Defer freeAddrinfo([&]() { freeaddrinfo(addr); });

    int sockFd = socket(addr->ai_family, addr->ai_socktype, addr->ai_protocol);
    assert(sockFd != -1);
    Defer closeSocket([&]() { close(sockFd); });

    int status = connect(sockFd, addr->ai_addr, addr->ai_addrlen);
    if (status != 0)
        throw NetworkException("cannot connect()");

    // Prepare a GET message
    String getRequest =
            String("GET /") + url.Path +
            " HTTP/1.1\r\n"
            "Host: " + url.Host + " \r\n" +
            "User-Agent: LinuxGetUrl/2.0 zackLt@umich.edu (Linux)\r\n"
            "Accept-Language: en-US,en;q=0.5\r\n"
            "Accept: text/html\r\n"
            "Accept-Encoding: identity\r\n"
            "Connection: close\r\n\r\n";

    String response;
    char buffer[10240];
    if (strcmp(url.Service, "http") == 0) {
        for (int sent = 0; sent < getRequest.size();) {
            int n = send(sockFd, getRequest.data() + sent, getRequest.size() - sent, 0);
            if (n <= 0)
                throw NetworkException();
            sent += n;
        }

        int got;
        while ((got = recv(sockFd, buffer, sizeof(buffer), 0)))
            response += String(buffer, got);
    } else if (strcmp(url.Service, "https") == 0) {
        SSL_library_init();
        SSL_CTX *ctx = SSL_CTX_new(TLS_method());
        SSL *ssl = SSL_new(ctx);

        SSL_set_fd(ssl, sockFd);
        SSL_connect(ssl);

        Defer deferDestructSSL([&]() {
            SSL_shutdown(ssl);
            SSL_free(ssl);
            SSL_CTX_free(ctx);
        });

        for (int sent = 0; sent < getRequest.size();) {
            int n = SSL_write(ssl, getRequest.data() + sent, getRequest.size() - sent);
            if (n <= 0)
                throw NetworkException();
            sent += n;
        }

        int got;
        while ((got = SSL_read(ssl, buffer, sizeof(buffer))))
            response += String(buffer, got);
    } else {
        throw std::runtime_error("protocol not supported");
    }

    return response;
}


ParsedHttpResponse FetchUrl(String url) {
    // Webpage that redirects too many times are either ads-clickfarms, or
    // malfunctioning. We only redirect for a generous MAX_REDIRECTION number
    // of times and stop following after that.
    for (int nRedirect = 0; nRedirect < MAX_REDIRECTION; nRedirect++) {
        ParsedUrl parsedUrl(url.cstr());
        String resp = HttpGet(parsedUrl);
        ParsedHttpResponse parsedResp(resp);
        if (parsedResp.StatusCode == "200")
            return parsedResp;
        if (parsedResp.StatusCode == "301" || parsedResp.StatusCode == "307" || parsedResp.StatusCode == "308")
            url = parsedResp.Location;
        else
            throw HttpRequestError("status code not handled");
        // TODO: should we, and how do we, handle HTTP 50x?
    }

    throw TooManyRedirectionError();
}

// ----------------------------------- PARSER CODE -----------------------------------
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


char *ReadFile( char *filename, size_t &fileSize )
   {
   // Read the file into memory.
   // You'll soon learn a much more efficient way to do this.

   // TODO: Replace with nmap
   // Attempt to Create an istream and seek to the end
   // to get the size.
   std::ifstream ifs( filename, std::ios::ate | std::ios::binary );
   if ( !ifs.is_open( ) )
      return nullptr;
   fileSize = ifs.tellg( );

   // Allocate a block of memory big enough to hold it.
   char *buffer = new char[ fileSize ];

   // Seek back to the beginning of the file, read it into
   // the buffer, then return the buffer.
   ifs.seekg( 0 );
   ifs.read( buffer, fileSize );
   return buffer;
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

    // TODO: convert this into the main controller thread that will be
    //  calling a url on this crawler.
    auto httpResp = FetchUrl(argv[2]);
    // auto httpResp = GetHtml("http://anglesharp.azurewebsites.net/Chunked"); // test chunked
    //std::cout << "code: " << httpResp.StatusCode << '\n'
    //          << "location: " << httpResp.Location << '\n'
    //          << "lang: " << httpResp.Language << '\n'
    //          << "---- body ----" << '\n'
    //          << httpResp.Body;
    std::cout << httpResp.Body;

   //size_t fileSize = 0;
   //char *buffer = ReadFile( argv[ 1 + ( argc == 3 ) ], fileSize );
   //if ( !buffer )
   //   {
   //   std::cerr << "Could not open the file." << std::endl;
   //   exit( 1 );
   //   }

   HtmlParser parser( httpResp.Body );
   PrintParse( parser );

   return 0;
}
