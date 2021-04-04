//
// Created by youh on 4/3/21.
//

#ifndef SCOOBLE_SEARCH_DEVOPS_H
#define SCOOBLE_SEARCH_DEVOPS_H

#include <unistd.h>
#include <wait.h>

#include "String.h"

void DiscordAlert( const scb::String &errMsg )
   {
   pid_t child = fork();
   if ( child != 0 )
      {
      // I'm parent
      waitpid( child, nullptr, 0 );
      return;
      }

   if ( execlp( "./discord-alert", "discord-alert", errMsg.cstr(), nullptr ) ==
        -1 )
      {
      std::cerr << "exec(): " << strerror( errno ) << std::endl;
      exit( 1 );
      }
   }

#endif // SCOOBLE_SEARCH_DEVOPS_H
