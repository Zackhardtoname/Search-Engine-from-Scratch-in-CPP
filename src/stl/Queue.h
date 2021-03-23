//
// Created by youh on 3/21/21.
//

#ifndef SCOOBLE_SEARCH_QUEUE_H
#define SCOOBLE_SEARCH_QUEUE_H

#include "Vector.h"

namespace scb
   {
template <typename T> class Queue
   {
      T *data = nullptr;
      int start = 0, sz = 0, capacity = 0;

   public:
      int size()
         {
         return sz;
         }

      bool empty()
         {
         return !sz;
         }

      T &front()
         {
         return data[ start ];
         }

      void popFront()
         {
         data[ start ].~T();

         start = ( start + 1 ) % capacity;
         sz--;
         }

      void pushBack( const T &val )
         {
         if ( sz == capacity )
            {
            // allocate new array
            int newCapacity = capacity * 2 + 1;
            auto *newData =
               reinterpret_cast<T *>( new char[ sizeof( T ) * newCapacity ] );

            // move old array to new array
            for ( int i = 0;  i < sz;  i++ )
               {
               T *old = data + ( start + i ) % capacity;
               new ( newData + i ) T( std::move( *old ) );
               old->~T();
               }

            // deallocate and replace old array
            delete[] reinterpret_cast<char *>( data );
            data = newData;

            // bookkeeping, size unchanged
            start = 0;
            capacity = newCapacity;
            }

         int back = ( start + sz ) % capacity;
         new ( data + back ) T( val );
         sz++;
         }
   };
   } // namespace scb

#endif // SCOOBLE_SEARCH_QUEUE_H
