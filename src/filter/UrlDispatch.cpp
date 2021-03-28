//
// Created by youh on 3/20/21.
//

#include "UrlDispatch.h"

#include <cassert>
#include <fstream>
#include <iostream>

#include "../stl/String.h"

UrlDispatch::UrlDispatch( const char *configFile )
   {
   std::ifstream f( configFile );
   assert( !!f && "cannot open config file" );

   int nPeers;
   f >> nPeers;
   std::cout << "Dispatching URLs to " << nPeers << " peers" << std::endl;

   scb::String s;
   std::cin >>
   }
