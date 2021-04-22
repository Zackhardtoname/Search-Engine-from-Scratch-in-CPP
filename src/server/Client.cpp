// A dummy client to test the server
// Client side C/C++ program to demonstrate Socket programming
// (inspiration: https://www.geeksforgeeks.org/socket-programming-cc/)
// Usage: run the server first, and then the client

#include <arpa/inet.h>
#include <iostream>
#include <sstream>
#include <stdio.h>
#include <sys/socket.h>
#include <unistd.h>

#define PORT 8080

int main( int argc, char const *argv[] )
   {
   int sock = 0, valread;
   struct sockaddr_in serv_addr;

   // Send a GET message.
   std::string end_char = "\r\n";
   std::string origin_info = "zackLT@umich.edu (Linux)";
   std::stringstream ss;
   ss << "GET /"
      << "index.html"
      << " HTTP/1.1" << end_char << "Host: "
      << "zackLight.com" << end_char << "User-Agent: LinuxGetUrl/2.0 "
      << origin_info << end_char << "Accept: */*" << end_char
      << "Accept-Encoding: identity" << end_char << "Connection: close"
      << end_char << end_char;

   char buffer[ 1024 ] = { 0 };
   if ( ( sock = socket( AF_INET, SOCK_STREAM, 0 ) ) < 0 )
      {
      printf( "\n Socket creation error \n" );
      return -1;
      }

   serv_addr.sin_family = AF_INET;
   serv_addr.sin_port = htons( PORT );

   // Convert IPv4 and IPv6 addresses from text to binary form
   if ( inet_pton( AF_INET, "127.0.0.1", &serv_addr.sin_addr ) <= 0 )
      {
      printf( "\nInvalid address/ Address not supported \n" );
      return -1;
      }

   if ( connect( sock, (struct sockaddr *)&serv_addr, sizeof( serv_addr ) ) <
        0 )
      {
      printf( "\nConnection Failed \n" );
      return -1;
      }
   send( sock, ss.str().c_str(), ss.str().size(), 0 );
   printf( "request sent\n" );
   valread = read( sock, buffer, 1024 );
   printf( "%s\n", buffer );
   return 0;
   }
