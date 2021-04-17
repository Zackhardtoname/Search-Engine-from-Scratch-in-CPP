#include "QueryParser.h"

int main()
   {
   Parser parser;
   std::string s;
   std::getline( std::cin, s );
   scb::String ss = s.c_str();

   auto ast = parser( ss );
   ast->print();
   std::cout << 