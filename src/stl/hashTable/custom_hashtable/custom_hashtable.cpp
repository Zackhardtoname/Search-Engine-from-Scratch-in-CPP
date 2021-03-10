#include <cstring>
#include <iostream>
#include <iterator>
#include <stdexcept>
bool CompareEqual( const char *L, const char *R ){
    while (*L==*R)
        if (*(L++) = '\0')
            return true;
    return false;
};


template< typename Key, typename Value > class Tuple
   {
   public:
      Key key;
      Value value;

      Tuple( const Key &k, const Value v ) : key( k ), value( v )
         {
         }
   };


template< typename Key, typename Value > class Bucket
   {
   public:
      Bucket *next;
      uint32_t hashValue;
      Tuple< Key, Value > tuple;

      Bucket( const Key &k, uint32_t h, const Value v ) :
            tuple( k, v ), next( nullptr ), hashValue( h )
         {
         }
   };
// first use a vector
template <typename T> class vector
   {
      static constexpr int GROWTH_FACTOR = 4;

   public:
      ~vector()
         {
         for ( uint i = 0;  i < vectorSize;  i++ )
            array[ i ].~T();
         delete[] reinterpret_cast<char *>( array );
         }

      vector() noexcept
         {
         array =
            reinterpret_cast<T *>( new char[ reservedSize * sizeof( T ) ] );
         }

      explicit vector( uint n ) : reservedSize( n ), vectorSize( n )
         {
         array =
            reinterpret_cast<T *>( new char[ reservedSize * sizeof( T ) ] );
         for ( uint i = 0;  i < n;  i++ )
            new ( array + i ) T();
         }

      vector( uint n, const T &val ) : reservedSize( n ), vectorSize( n )
         {
         array =
            reinterpret_cast<T *>( new char[ reservedSize * sizeof( T ) ] );
         for ( uint i = 0;  i < n;  i++ )
            new ( array + i ) T( val );
         }

      vector( const T *first, const T *last )
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

      vector( std::initializer_list<T> L ) : reservedSize( L.size() )
         {
         array =
            reinterpret_cast<T *>( new char[ reservedSize * sizeof( T ) ] );
         for ( auto &v : L )
            {
            new ( array + vectorSize ) T( v );
            vectorSize++;
            }
         }

      vector( const vector<T> &other )
          : reservedSize( other.reservedSize ), vectorSize( other.vectorSize )
         {
         array =
            reinterpret_cast<T *>( new char[ reservedSize * sizeof( T ) ] );
         for ( uint i = 0;  i < other.vectorSize;  i++ )
            new ( array + i ) T( other[ i ] );
         }

      vector( vector<T> &&temp ) noexcept
          : reservedSize( temp.reservedSize ), vectorSize( temp.vectorSize ),
            array( temp.array )
         {
         temp.array = nullptr;
         temp.vectorSize = 0;
         temp.reservedSize = 0;
         }

      vector<T> &operator=( const vector<T> &other )
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
            delete[] array;
            // 4. bookkeeping
            array = reinterpret_cast<T *>( newArray );
            vectorSize = other.vectorSize;

            // AUTOGRADER-SPECIFIC
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

      vector<T> &operator=( vector<T> &&temp ) noexcept
         {
         // 1. Deconstruct what I have
         for ( uint i = 0;  i < vectorSize;  i++ )
            reinterpret_cast<T *>( array )[ i ].~T();
         delete[] array;

         // 2. Snatch temp
         this->array = temp.array;
         this->vectorSize = temp.vectorSize;
         this->reservedSize = temp.reservedSize;

         temp.array = nullptr;
         temp.vectorSize = 0;
         temp.reservedSize = 0;

         return *this;
         }

      vector<T> &operator=( std::initializer_list<T> L )
         {
         if ( vectorSize == reservedSize )
            reserve( reservedSize * GROWTH_FACTOR + 1 );
         vectorSize = 0;
         for ( auto &v : L )
            array[ vectorSize++ ] = v;
         return *this;
         }

      bool operator==( const vector<T> &vec ) const
         {
         if ( vectorSize != vec.vectorSize )
            return false;
         for ( uint i = 0;  i < vectorSize;  ++i )
            if ( array[ i ] != vec.array[ i ] )
               return false;
         return true;
         }

      bool operator!=( const vector<T> &vec ) const
         {
         if ( vectorSize != vec.vectorSize )
            return true;
         for ( uint i = 0;  i < vectorSize;  ++i )
            if ( array[ i ] != vec.array[ i ] )
               return true;
         return false;
         }

      bool operator<( const vector<T> &vec ) const
         {
         auto _min = std::min( vectorSize, vec.vectorSize );
         for ( uint i = 0;  i < _min;  ++i )
            if ( array[ i ] != vec.array[ i ] )
               return array[ i ] < vec.array[ i ];
         return vectorSize < vec.vectorSize;
         }

      bool operator<=( const vector<T> &vec ) const
         {
         auto _min = std::min( vectorSize, vec.vectorSize );
         for ( uint i = 0;  i < _min;  ++i )
            if ( array[ i ] != vec.array[ i ] )
               return array[ i ] < vec.array[ i ];
         return vectorSize <= vec.vectorSize;
         }

      bool operator>( const vector<T> &vec ) const
         {
         auto _min = std::min( vectorSize, vec.vectorSize );
         for ( uint i = 0;  i < _min;  ++i )
            if ( array[ i ] != vec.array[ i ] )
               return array[ i ] > vec.array[ i ];
         return vectorSize > vec.vectorSize;
         }

      bool operator>=( const vector<T> &vec ) const
         {
         auto _min = std::min( vectorSize, vec.vectorSize );
         for ( uint i = 0;  i < _min;  ++i )
            if ( array[ i ] != vec.array[ i ] )
               return array[ i ] > vec.array[ i ];
         return vectorSize >= vec.vectorSize;
         }

      T &operator[]( uint loc )
         {
         return array[ loc ];
         }

      const T &operator[]( uint loc ) const
         {
         return array[ loc ];
         }

      T &at( uint loc )
         {
         if ( loc < vectorSize )
            return array[ loc ];
         throw std::range_error( "range_error" );
         }

      const T &at( uint loc ) const
         {
         if ( loc < vectorSize )
            return array[ loc ];
         throw std::range_error( "range_error" );
         }

      T &front()
         {
         return array[ 0 ];
         }

      const T &front() const
         {
         return array[ 0 ];
         }

      T &back()
         {
         return array[ vectorSize - 1 ];
         }

      const T &back() const
         {
         return array[ vectorSize - 1 ];
         }

      T *begin() noexcept
         {
         return array;
         }

      T *end() noexcept
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

      std::reverse_iterator<T *> rbegin() noexcept
         {
         return std::reverse_iterator<T *>( array + vectorSize );
         }

      std::reverse_iterator<T *> rend() noexcept
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

      void shrink_to_fit()
         {
         reservedSize = vectorSize;
         reallocate();
         }

      T *data() noexcept
         {
         return array;
         }

      const T *data() const noexcept
         {
         return array;
         }

      template <class... Args> void emplace_back( Args &&...args )
         {
         if ( vectorSize == reservedSize )
            reserve( GROWTH_FACTOR * reservedSize + 1 );
         new ( array + vectorSize ) T( std::forward<Args>( args )... );
         vectorSize++;
         }

      void push_back( const T &val )
         {
         if ( vectorSize == reservedSize )
            reserve( reservedSize * GROWTH_FACTOR + 1 );
         new ( array + vectorSize ) T( val );
         vectorSize++;
         }

      void push_back( T &&val )
         {
         if ( vectorSize == reservedSize )
            reserve( reservedSize * GROWTH_FACTOR + 1 );
         new ( array + vectorSize ) T( val );
         vectorSize++;
         }

      void pop_back()
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

      void swap( vector<T> &vec )
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

template <typename T> class LinkedList
   {
   private:
      class Node
         {
         public:
            T data;
            Node *next;
            Node( T val ) : data( val ), next( nullptr ){};
            Node( T val, Node *node ) : data( val ), next( node ){};
         };

   public:
      LinkedList()
         {
         front = nullptr;
         }

      LinkedList( const LinkedList &list )
         {
         copier( list );
         }

      ~LinkedList()
         {
         deletor();
         }

      LinkedList &operator=( const LinkedList &list )
         {
         if ( this != &list )
            deletor, copier( list );
         return *this;
         }

      Node *find( T val ) const
         {
         auto tmp = front;
         while ( tmp )
            {
            if ( tmp->data == val )
               return tmp;
            tmp = tmp->next;
            }
         return nullptr;
         }

      Node *find_next( T val ) const
         {
         auto tmp = front;
         while ( tmp->next )
            {
            if ( tmp->next->data == val )
               return tmp;
            tmp = tmp->next;
            }
         return nullptr;
         }

      bool exists( T val ) const
         {
         return find( val ) != nullptr;
         }

      bool insert( T val )
         {
         if ( !exists( val ) )
            {
            front = new Node( val, front );
            return true;
            }
         return false;
         }

      bool remove( T val )
         {
         if ( front == nullptr )
            return false;
         else if ( front->data == val )
            {
            front = front->next;
            return true;
            }

         auto tmp = find_next( val );

         if ( tmp == nullptr )
            return false;

         auto toDelete = tmp->next;
         tmp->next = tmp->next->next;
         delete toDelete;
         return true;
         }

      vector<T> get_data() const
         {
         vector<T> storage;
         for ( auto tmp = front;  tmp;  tmp = tmp->next )
            storage.push_back( tmp->data );
         return storage;
         }

   private:
      Node *front;
      // makes deepcopy
      void copier( const LinkedList &list )
         {
         front = nullptr;
         Node *OG, *CP;
         if ( list.front )
            OG = list.front, CP = new Node( OG->data );
         front = CP;
         OG = OG->next;
         while ( OG )
            CP->next = new Node( OG->data ), CP = CP->next, OG = OG->next;
         }

      // deletion
      void deletor()
         {
         Node *tmp;
         while ( front )
            tmp = front->next, delete front, front = tmp;
         }
   };

template <typename T> class HashTable
   {
   private:
      class Node
         {
         public:
            T data;
            Node *next;
            Node( T val ) : data( val ), next( nullptr ){};
            Node( T val, Node *node ) : data( val ), next( node ){};
         };

   public:
      ~HashTable()
         {
         delete[] data;
         }

      HashTable()
         {
         //"small" prime number
         hasherSize = 97;
         // initialize it
         data = new LinkedList[ hasherSize ], length = 0;
         }

      HashTable( int size )
         {
         auto primality = []( int a ) int ->
         {
               for ( int i = 2;  i < a;  ++i )
                  if ( !( a % i ) )
                     return 0;
               return 1;
         };
         while ( !primality( ++size ) )
            ;
         hasherSize = size;
         data = new LinkedList[ hasherSize ], length = 0;
         }

      HashTable( const HashTable &h )
         {
         copier( h );
         }

      HashTable &operator=( const HashTable &ht )
         {
         if ( this != &ht )
            {
            if ( !ht.data )
               delete[] data;
            copier( ht );
            }
         }

      //fun tiny obfuscation
      bool insert(const T&val){
         auto tryInsert = data[hashFunction(val)].insert(val);
         return length+=tryInsert, tryInsert;
      }

      bool remove(const T& val){
         auto tryRemove = data[hashFunction(val)].remove(val);
         return length-=tryRemove, tryRemove;
      }

      bool exists(const T& val) const{
         return data[hashFunction(val)].exists(val);
      }

      Tuple<Key, Value> Find(const Key k, const Value initialValue = nullptr){
         auto finder = data[hashFunction(key)].find(val);
         
      }

      int size() const{
         return length;
      }

      int capacity() const{
         return hasherSize;
      }

      //load factor func from 281
      int lF() const{
         return hasherSize/length;
      }

      /*
      to implement
            instersections, unions, differences, set operations (will be helpful to compare maps wihtout going throug everythign)
      */

   private : 
         //really only works for string right now lmao, thinking of making it VarArg like printf
         //currently just using horners
         int hashFunction(const T&val) const{
            int value = 0;
            while(*val)
               value = (value * 32 + *val > 96 ? *val -96 : *(val) - 64)%hasherSize, ++val;
            return value;
         }

         void copier(const HashTable &ht){
            if (ht.data){
               length=ht.size(), hasherSize=ht.capacity();
               data = new LinkedList[hasherSize];
               for (int i = 0; i<hasherSize; ++i)
                  data[i] = ht.data[i];
            }
         }

         LinkedList *data;
         int hasherSize;
         int length;
   };