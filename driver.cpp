/*
 * driver.cpp
 *
 * Driver for the math parser program
 *
 * Lab3: You do not have to modify this file, but you may choose to do so
 *
 * If you do, note that your output is expected to match ours
 */

#include <iostream>

#include "expression.h"
#include "parser.h"

int main( )
   {
   std::string input;
   std::getline( std::cin, input );

   Parser parser( input );
   auto j = parser.Parse();
   std::cout << j << "\n";
   }
