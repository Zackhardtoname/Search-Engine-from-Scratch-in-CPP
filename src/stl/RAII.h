//
// Created by youh on 3/10/21.
//

#ifndef SCOOBLE_SEARCH_RAII_H
#define SCOOBLE_SEARCH_RAII_H

#include <cassert>
#include <cerrno>
#include <functional>
#include <unistd.h>
#include <wait.h>

class Defer
   {
      std::function<void( void )> deferredAction;

   public:
      explicit Defer( std::function<void( void )> action )
          : deferredAction( std::move( action ) )
         {
         }

      ~Defer()
         {
         deferredAction();
         }
   };

class ForkedChild
   {
      bool waited = false;

   public:
      const pid_t pid;

      ForkedChild() : pid( fork() )
         {
         assert( pid != -1 && "fork() failed" );
         }

      ~ForkedChild()
         {
         if ( pid == 0 )
            return;
         if ( !waited )
            Wait();
         }

      [[nodiscard]] bool IsChild() const
         {
         return pid == 0;
         }

      int Wait()
         {
         while ( true )
            {
            int stat;
            pid_t dead = waitpid( pid, &stat, 0 );
            if ( dead == -1 )
               {
               if ( errno != EINTR )
                  assert( !"waitpid() failed" );
               }
            else
               {
               assert( dead == pid );
               waited = true;
               return WEXITSTATUS( stat );
               }
            }
         }
   };

#endif // SCOOBLE_SEARCH_RAII_H
