// snippet from: https://stackoverflow.com/a/7577532

#include <chrono>
#include <iostream>
#include <random>
#include <thread>
#include <unistd.h>

int main( int argc, char *argv[] )
   {
   std::cout << getpid() << " alive" << '\n';

   std::cout << getpid() << " received ";
   for ( int i = 0;  i < argc;  ++i )
      std::cout << argv[ i ] << ' ';
   std::cout << std::endl;

   // This is needed along with the next seed to get true randomness for each
   // run
   srand( time( NULL ) );
   std::mt19937_64 eng{ std::random_device{}() }; // or seed however you want
   std::uniform_int_distribution<> dist{ 5, 10 };
   std::this_thread::sleep_for( std::chrono::seconds{ dist( eng ) } );

   std::cout << getpid() << " died after " << dist( eng ) << " sec";

   // random bad return code, conveniently indicating time slept
   return dist( eng );
   }