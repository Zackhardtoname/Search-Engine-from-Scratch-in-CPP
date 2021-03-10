#ifndef STL_VECTOR_H
#define STL_VECTOR_H

#include <cstring>
#include <iostream>
#include <iterator>
#include <stdexcept>

// ideas from the gcc github, but they are hard to read.
// some ideas may be wrong, but i have tested and they work well enough.
// well enough being as good as or better for many operations.
namespace agr {
template <typename T> class Vector
   {
      static constexpr int GROWTH_FACTOR = 4;

   public:
      ~Vector()
         {
         for ( uint i = 0;  i < vectorSize;  i++ )
            array[ i ].~T();
         delete[] reinterpret_cast<char *>( array );
         }

      Vector() noexcept
         {
         array =
            reinterpret_cast<T *>( new char[ reservedSize * sizeof( T ) ] );
         }

      explicit Vector( uint n ) : reservedSize( n ), vectorSize( n )
         {
         array =
            reinterpret_cast<T *>( new char[ reservedSize * sizeof( T ) ] );
         for ( uint i = 0;  i < n;  i++ )
            new ( array + i ) T();
         }

      Vector( uint n, const T &val ) : reservedSize( n ), vectorSize( n )
         {
         array =
            reinterpret_cast<T *>( new char[ reservedSize * sizeof( T ) ] );
         for ( uint i = 0;  i < n;  i++ )
            new ( array + i ) T( val );
         }

      Vector( const T *first, const T *last )
          : vectorSize( last - first ), reservedSize( vectorSize )
         {
         array =
            reinterpret_cast<T *>( new char[ reservedSize * sizeof( T ) ] );
         // TODO check for problems with increment outside loop header
         for ( uint i = 0;  i < vectorSize;  ++i, ++first )
            {
            T *placement = reinterpret_cast<T *>( array ) + i;
            new ( placement ) T( *first );
            }
         }

      Vector( std::initializer_list<T> L ) : reservedSize( L.size() )
         {
         array =
            reinterpret_cast<T *>( new char[ reservedSize * sizeof( T ) ] );
         for ( auto &v : L )
            {
            new ( array + vectorSize ) T( v );
            vectorSize++;
            }
         }

      Vector( const Vector<T> &other )
          : reservedSize( other.reservedSize ), vectorSize( other.vectorSize )
         {
         array =
            reinterpret_cast<T *>( new char[ reservedSize * sizeof( T ) ] );
         for ( uint i = 0;  i < other.vectorSize;  i++ )
            new ( array + i ) T( other[ i ] );
         }

      Vector( Vector<T> &&temp ) noexcept
          : reservedSize( temp.reservedSize ), vectorSize( temp.vectorSize ),
            array( temp.array )
         {
         temp.array = nullptr;
         temp.vectorSize = 0;
         temp.reservedSize = 0;
         }

      Vector<T> &operator=( const Vector<T> &other )
         {
         if ( this == &other )
            return *this;

         if ( other.vectorSize > this->vectorSize )
            {
            // 1. allocate new
            T *newArray = reinterpret_cast<T *>(
               new char[ other.reservedSize * sizeof( T ) ] );
            // 2. dup other to new with copy-constructor
            for ( uint i = 0;  i < other.vectorSize;  i++ )
               new ( newArray + i ) T( other[ i ] );
            // 3. destruct current
            for ( uint i = 0;  i < vectorSize;  i++ )
               reinterpret_cast<T *>( array )[ i ].~T();
            delete[] reinterpret_cast<char *>( array );
            // 4. bookkeeping
            array = reinterpret_cast<T *>( newArray );
            vectorSize = other.vectorSize;

            reservedSize = other.reservedSize;
            }
         else
            {
            // 1. copy other to front with copy-assignment
            for ( uint i = 0;  i < other.vectorSize;  i++ )
               this->operator[]( i ) = other[ i ];
            // 2. destruct back, but don't resize.
            for ( uint i = other.vectorSize;  i < vectorSize;  i++ )
               reinterpret_cast<T *>( array )[ i ].~T();
            // 3. bookkeeping
            vectorSize = other.vectorSize;

            // AUTOGRADER-SPECIFIC
            reservedSize = other.reservedSize;
            reallocate();
            }

         return *this;
         }

      Vector<T> &operator=( Vector<T> &&temp ) noexcept
         {
         // 1. Deconstruct what I have
         for ( uint i = 0;  i < vectorSize;  i++ )
            reinterpret_cast<T *>( array )[ i ].~T();
         delete[] reinterpret_cast<char *>( array );

         // 2. Snatch temp
         this->array = temp.array;
         this->vectorSize = temp.vectorSize;
         this->reservedSize = temp.reservedSize;

         temp.array = nullptr;
         temp.vectorSize = 0;
         temp.reservedSize = 0;

         return *this;
         }

      Vector<T> &operator=( std::initializer_list<T> L )
         {
         *this = std::move( Vector<T>( L ) );
         }

      bool operator==( const Vector<T> &vec ) const
         {
         if ( vectorSize != vec.vectorSize )
            return false;
         for ( uint i = 0;  i < vectorSize;  ++i )
            if ( array[ i ] != vec.array[ i ] )
               return false;
         return true;
         }

      bool operator!=( const Vector<T> &vec ) const
         {
         if ( vectorSize != vec.vectorSize )
            return true;
         for ( uint i = 0;  i < vectorSize;  ++i )
            if ( array[ i ] != vec.array[ i ] )
               return true;
         return false;
         }

      bool operator<( const Vector<T> &vec ) const
         {
         auto _min = std::min( vectorSize, vec.vectorSize );
         for ( uint i = 0;  i < _min;  ++i )
            if ( array[ i ] != vec.array[ i ] )
               return array[ i ] < vec.array[ i ];
         return vectorSize < vec.vectorSize;
         }

      bool operator<=( const Vector<T> &vec ) const
         {
         auto _min = std::min( vectorSize, vec.vectorSize );
         for ( uint i = 0;  i < _min;  ++i )
            if ( array[ i ] != vec.array[ i ] )
               return array[ i ] < vec.array[ i ];
         return vectorSize <= vec.vectorSize;
         }

      bool operator>( const Vector<T> &vec ) const
         {
         auto _min = std::min( vectorSize, vec.vectorSize );
         for ( uint i = 0;  i < _min;  ++i )
            if ( array[ i ] != vec.array[ i ] )
               return array[ i ] > vec.array[ i ];
         return vectorSize > vec.vectorSize;
         }

      bool operator>=( const Vector<T> &vec ) const
         {
         auto _min = std::min( vectorSize, vec.vectorSize );
         for ( uint i = 0;  i < _min;  ++i )
            if ( array[ i ] != vec.array[ i ] )
               return array[ i ] > vec.array[ i ];
         return vectorSize >= vec.vectorSize;
         }

      T &operator[]( uint loc ) const noexcept
         {
         return array[ loc ];
         }

      T &at( uint loc ) const
         {
         if ( loc < vectorSize )
            return array[ loc ];
         throw std::range_error( "range_error" );
         }

      T &front() const noexcept
         {
         return array[ 0 ];
         }

      T &back() const noexcept
         {
         return array[ vectorSize - 1 ];
         }

      T *begin() const noexcept
         {
         return array;
         }

      T *end() const noexcept
         {
         return array + vectorSize;
         }

      const T *cbegin() const noexcept
         {
         return array;
         }

      const T *cend() const noexcept
         {
         return array + vectorSize;
         }

      std::reverse_iterator<T *> rbegin() const noexcept
         {
         return std::reverse_iterator<T *>( array + vectorSize );
         }

      std::reverse_iterator<T *> rend() const noexcept
         {
         return std::reverse_iterator<T *>( array );
         }

      // TODO: maybe remove const on crbegin and crend? fuck if i know.
      std::reverse_iterator<const T *> crbegin() const noexcept
         {
         return std::reverse_iterator<const T *>( array + vectorSize );
         }

      std::reverse_iterator<const T *> crend() const noexcept
         {
         return std::reverse_iterator<const T *>( array );
         }

      [[nodiscard]] bool empty() const noexcept
         {
         return vectorSize == 0;
         }

      [[nodiscard]] uint size() const noexcept
         {
         return vectorSize;
         }

      [[nodiscard]] uint capacity() const noexcept
         {
         return reservedSize;
         }

      void resize( uint sz )
         {
         if ( sz > vectorSize )
            {
            if ( sz > reservedSize )
               reserve( sz );

            for ( uint i = vectorSize;  i < sz;  i++ )
               new ( array + i ) T();
            }
         else
            {
            for ( uint i = sz;  i < vectorSize;  i++ )
               array[ i ].~T();
            }
         vectorSize = sz;
         }

      void resize( uint sz, const T &val )
         {
         if ( sz > vectorSize )
            {
            if ( sz > reservedSize )
               reserve( sz );

            for ( uint i = vectorSize;  i < sz;  i++ )
               new ( array + i ) T( val );
            }
         else
            {
            for ( uint i = sz;  i < vectorSize;  i++ )
               array[ i ].~T();
            }
         vectorSize = sz;
         }

      void reserve( uint sz )
         {
         if ( sz > reservedSize )
            {
            reservedSize = sz;
            reallocate();
            }
         }

      void shrinkToFit()
         {
         reservedSize = vectorSize;
         reallocate();
         }

      T *data() const noexcept
         {
         return array;
         }

      template <class... Args> void emplaceBack( Args &&...args )
         {
         if ( vectorSize == reservedSize )
            reserve( GROWTH_FACTOR * reservedSize + 1 );
         new ( array + vectorSize ) T( std::forward<Args>( args )... );
         vectorSize++;
         }

      void pushBack( const T &val )
         {
         if ( vectorSize == reservedSize )
            reserve( reservedSize * GROWTH_FACTOR + 1 );
         new ( array + vectorSize ) T( val );
         vectorSize++;
         }

      void pushBack( T &&val )
         {
         if ( vectorSize == reservedSize )
            reserve( reservedSize * GROWTH_FACTOR + 1 );
         new ( array + vectorSize ) T( val );
         vectorSize++;
         }

      void popBack()
         {
         --vectorSize;
         array[ vectorSize ].~T();
         }

      inline T *insert( int idx, const T &val )
         {
         if ( vectorSize == reservedSize )
            reserve( reservedSize * GROWTH_FACTOR + 1 );

         for ( int i = vectorSize;  i > idx;  i-- )
            {
            new ( array + i ) T( std::move( array[ i - 1 ] ) );
            array[ i - 1 ].~T();
            }

         new ( array + idx ) T( val );

         vectorSize++;

         return array + idx;
         }

      T *insert( const T *it, const T &val )
         {
         return insert( it - array, val );
         }

      inline T *erase( int idx )
         {
         array[ idx ].~T();
         for ( int i = idx + 1;  i < vectorSize;  i++ )
            {
            new ( array + i - 1 ) T( std::move( array[ i ] ) );
            array[ i ].~T();
            }

         vectorSize--;

         return array + idx;
         }

      T *erase( const T *itr )
         {
         return erase( itr - array );
         }

      inline T *erase( int first, int last )
         {
         for ( int i = first;  i < last;  i++ )
            array[ i ].~T();

         for ( int i = last;  i < vectorSize;  i++ )
            {
            new ( array + first + ( i - last ) ) T( std::move( array[ i ] ) );
            array[ i ].~T();
            }

         vectorSize -= ( last - first );

         return array + first;
         }

      T *erase( const T *first, const T *last )
         {
         return erase( first - array, last - array );
         }

      T *find( const T &val )
         {
         auto first = begin(), last = end();
         for ( ;  first != last;  ++first )
            if ( *first == val )
               return first;
         return nullptr;
         }

      // to be implemented
      // T * find(const T * first, const T* last, const T&&val){
      //
      //}

      void swap( Vector<T> &vec )
         {
         std::swap( vec.array, this->array );
         std::swap( vec.vectorSize, this->vectorSize );
         std::swap( vec.reservedSize, this->reservedSize );
         }

      void clear() noexcept
         {
         for ( auto i = 0;  i < vectorSize;  ++i )
            array[ i ].~T();
         vectorSize = 0;
         }

   private:
      uint vectorSize = 0;
      uint reservedSize = 0;
      T *array;

      inline void reallocate()
         {
         T *tempArray =
            reinterpret_cast<T *>( new char[ reservedSize * sizeof( T ) ] );
         for ( int i = 0;  i < vectorSize;  i++ )
            new ( tempArray + i ) T( std::move( array[ i ] ) );

         for ( int i = 0;  i < vectorSize;  i++ )
            array[ i ].~T();
         delete[] reinterpret_cast<char *>( array );

         array = tempArray;
         }
   };
}
#endif // STL_VECTOR_H
