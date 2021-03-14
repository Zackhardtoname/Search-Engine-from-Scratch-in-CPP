#ifndef UNIQUE_POINTER_H
#define UNIQUE_POINTER_H

#include "C20_PTR_FUNCTIONALITIES.h"

SINGLE_POINTER_TMPLT
class unique_ptr
   {
   public:
      ~unique_ptr() noexcept
         {
         auto ptr = impl.impl_ptr();
         auto &deletor = impl.impl_del();
         if ( ptr )
            deletor( ptr );
         }

      constexpr unique_ptr() noexcept = default;

      constexpr unique_ptr( std::nullptr_t ) noexcept
         {
         }

      explicit unique_ptr( T *pointer ) noexcept : impl{ pointer }
         {
         }

      unique_ptr( T *pointer,
                  typename std::conditional<std::is_reference<D>::value, D,
                                            const D &>::type Deletor ) noexcept
          : impl{ pointer, Deletor }
         {
         }

      unique_ptr( T *pointer,
                  typename std::remove_reference<D>::type &&Deletor ) noexcept
          : impl{ pointer, std::move( Deletor ) }
         {
         static_assert( !std::is_reference<D>::value );
         }

      unique_ptr( unique_ptr &&other ) noexcept
          : impl{ other.release(), std::forward<D>( other.get_deletor() ) }
         {
         }

      OTHEREX
      unique_ptr( unique_ptr<U, E> &&other ) noexcept
          : impl{ other.release(), std::forward<D>( other.get_deletor() ) }
         {
         }

      unique_ptr &operator=( unique_ptr &&other ) noexcept
         {
         reset( other.release() );
         auto &deletor = impl.impl_del();
         deletor = other.get_deletor();
         return *this;
         }

      OTHEREX
      unique_ptr &operator=( unique_ptr<U, E> &&other ) noexcept
         {
         reset( other.release() );
         auto &deletor = impl.impl_del();
         deletor = other.get_deletor();
         return *this;
         }

      unique_ptr &operator=( std::nullptr_t ) noexcept
         {
         reset();
         return *this;
         }

      // todo maybe crash if nullptr?
      T &operator*() const noexcept
         {
         auto ptr = impl.impl_ptr();

         return *ptr;
         }

      // todo maybe crash if nullptr?
      T *operator->() const noexcept
         {
         T *ptr = impl.impl_ptr();
         return ptr;
         }

      T *get() const noexcept
         {
         return impl.impl_ptr();
         }

      D &get_deletor() noexcept
         {
         return impl.impl_del();
         }

      const D &get_deletor() const noexcept
         {
         return impl.impl_del();
         }

      explicit operator bool() const noexcept
         {
         if ( impl.impl_ptr() )
            return true;
         return false;
         }

      T *release() noexcept
         {
         auto &ptr = impl.impl_ptr();
         T *other = ptr;
         ptr = nullptr;
         return other;
         }

      void reset( T *other ) noexcept
         {
         auto &ptr = impl.impl_ptr();
         auto &deletor = impl.impl_del();
         if ( ptr )
            deletor( ptr );
         ptr = other;
         }
      void reset() noexcept
         {
         auto &ptr = impl.impl_ptr();
         auto &deletor = impl.impl_del();
         if ( ptr )
            deletor( ptr );
         ptr = nullptr;
         }
      void swap( unique_ptr &other ) noexcept
         {
         std::swap( impl, other.impl );
         }

      // u need this, i cried for hours forgetting it
      // it stops def del func
      unique_ptr( const unique_ptr & ) = delete;

      // same
      unique_ptr &operator=( const unique_ptr & ) = delete;

   private:
      scb::base::ptr<T, D> impl;
   };

// TODO implement for arrays--basically same thing but array accesses

ARRAY_POINTER_TMPLT
class unique_ptr<T[], D>
   {
   public:
      ~unique_ptr() noexcept
         {
         auto ptr = impl.impl_ptr();
         auto &deletor = impl.impl_del();
         if ( ptr )
            deletor( ptr );
         }

      constexpr unique_ptr() noexcept = default;

      constexpr unique_ptr( std::nullptr_t ) noexcept
         {
         }

      explicit unique_ptr( T *pointer ) noexcept : impl{ pointer }
         {
         }

      unique_ptr( T *pointer,
                  typename std::conditional<std::is_reference<D>::value, D,
                                            const D &>::type Deletor ) noexcept
          : impl{ pointer, Deletor }
         {
         }

      unique_ptr( T *pointer,
                  typename std::remove_reference<D>::type &&Deletor ) noexcept
          : impl{ pointer, std::move( Deletor ) }
         {
         static_assert( !std::is_reference<D>::value );
         }

      unique_ptr( unique_ptr &&other ) noexcept
          : impl{ other.release(), other.get_deletor() }
         {
         }

      unique_ptr &operator=( unique_ptr &&other ) noexcept
         {
         reset( other.release() );
         auto &deletor = impl.impl_del();
         deletor = other.get_deletor();
         return *this;
         }

      unique_ptr &operator=( std::nullptr_t ) noexcept
         {
         reset();
         return *this;
         }

      // todo maybe crash if nullptr?
      T &operator[]( int idx ) const noexcept
         {
         auto ptr = impl.impl_ptr();

         return ptr[ idx ];
         }

      T *get() const noexcept
         {
         return impl.impl_ptr();
         }

      D &get_deletor() noexcept
         {
         return impl.impl_del();
         }

      const D &get_deletor() const noexcept
         {
         return impl.impl_del();
         }

      explicit operator bool() const noexcept
         {
         if ( impl.impl_ptr() )
            return true;
         return false;
         }

      T *release() noexcept
         {
         auto &ptr = impl.impl_ptr();
         T *other = ptr;
         ptr = nullptr;
         return other;
         }

      void reset( T *other ) noexcept
         {
         auto &ptr = impl.impl_ptr();
         auto &deletor = impl.impl_del();
         if ( ptr )
            deletor( ptr );
         ptr = other;
         }
      void reset() noexcept
         {
         auto &ptr = impl.impl_ptr();
         auto &deletor = impl.impl_del();
         if ( ptr )
            deletor( ptr );
         ptr = nullptr;
         }
      void swap( unique_ptr &other ) noexcept
         {
         std::swap( impl, other.impl );
         }

      // u need this, i cried for hours forgetting it
      // it stops def del func
      unique_ptr( const unique_ptr & ) = delete;

      // same
      unique_ptr &operator=( const unique_ptr & ) = delete;

   private:
      scb::base::ptr<T, D> impl;
   };

GENERIC
struct UI
   {
      using O = unique_ptr<T>;
   };

GENERIC
struct UI<T[]>
   {
      using U = unique_ptr<T[]>;
   };

SZSPEC
struct UI<T[ N ]>
   {
      using K = void;
   };

PARAMPACK
typename UI<T>::O make_unique( Args &&...args )
   {
   return unique_ptr<T>{ new T{ std::forward<Args>( args )... } };
   }

GENERIC
typename UI<T>::U make_unique( int sz )
   {
   return unique_ptr<T>{ new typename std::remove_extent<T>::type[ sz ] };
   }

PARAMPACK
typename UI<T>::K make_unique( Args &&... ) = delete;

VARTEMP
inline bool operator==( const unique_ptr<T, D> &first,
                        const unique_ptr<U, E> second )
   {
   return first.get() == second.get();
   }

VARTEMP
inline bool operator!=( const unique_ptr<T, D> &first,
                        const unique_ptr<U, E> second )
   {
   return first.get() != second.get();
   }

GENEX
inline bool operator==( const unique_ptr<T, D> &other, std::nullptr_t ) noexcept
   {
   return !other;
   }

GENEX
inline bool operator!=( const unique_ptr<T, D> &other, std::nullptr_t ) noexcept
   {
   // TODO both != operators might require exp bool functional cast ala bool{}
   return !!other;
   }

GENEX
inline bool operator==( std::nullptr_t, const unique_ptr<T, D> &other ) noexcept
   {
   return !other;
   }

GENEX
inline bool operator!=( std::nullptr_t, const unique_ptr<T, D> &other ) noexcept
   {
   return !!other;
   }

OUT_TMPLT
std::basic_ostream<E, T> &operator<<( std::basic_ostream<E, T> &os,
                                      const unique_ptr<Y> &ptr )
   {
   os << ptr.get();
   return os;
   }

GENEX
inline void swap( unique_ptr<T, D> &first, unique_ptr<T, D> &second )
   {
   first.swap( second );
   }

#endif
