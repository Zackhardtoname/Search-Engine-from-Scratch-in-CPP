#ifndef C20_PTR_FUNCTIONALITIES
#define C20_PTR_FUNCTIONALITIES

#define GENERIC template <typename T>
#define SINGLE_POINTER_TMPLT                                                   \
   template <typename T, typename D = scb::UNDEFINED_DELETE_PROTOCOL<T>>
#define ARRAY_POINTER_TMPLT template <typename T, typename D>
#define SZSPEC template <typename T, int N>
#define PARAMPACK template <typename T, typename... Args>
#define VARTEMP template <typename T, typename D, typename U, typename E>
#define GENEX template <typename T, typename D>
#define OUT_TMPLT template <class E, class T, class Y>
#define OTHER template <typename U>
#define OTHEREX template <typename U, typename E>
#define DEL_TMPLT template <typename Del>

#include <cassert>
#include <cstddef>
#include <functional>
#include <iostream>
#include <tuple>
#include <type_traits>
#include <utility>
namespace scb
   {

      ////////////////////////////////////////
      //// PSEUDO RAII FOR SMART POINTERS ////
      ////////////////////////////////////////

      GENERIC
      class UNDEFINED_DELETE_PROTOCOL
         {
         public:
            constexpr UNDEFINED_DELETE_PROTOCOL() noexcept = default;

            OTHER
            UNDEFINED_DELETE_PROTOCOL( const UNDEFINED_DELETE_PROTOCOL<U> & )
            noexcept
               {
               }

            void operator()( T *obj )
               {
               delete obj;
               }
         };

      // C++20 only, it looks like, but im not sure.

      GENERIC
      class UNDEFINED_DELETE_PROTOCOL<T[]>
         {
         public:
            constexpr UNDEFINED_DELETE_PROTOCOL() noexcept = default;

            OTHER
            UNDEFINED_DELETE_PROTOCOL( const UNDEFINED_DELETE_PROTOCOL<U[]> & )
            noexcept
               {
               }

            void operator()( T *obj )
               {
               delete[] obj;
               }
         };

      ////////////////////////////////////////
      //// DELETOR WRAPPER, BASE IMPL PTR ////
      ////////////////////////////////////////

      namespace base
         {

            GENEX
            class ptr
               {
               public:
                  constexpr ptr() noexcept = default;

                  ptr( T *pointer ) : impl_T{}
                     {
                     impl_ptr() = pointer;
                     }

                  DEL_TMPLT
                  ptr( T *pointer, Del &&deletor )
                      : impl_T( pointer, std::forward<Del>( deletor ) )
                     {
                     }

                  ~ptr() noexcept = default;

                  T *&impl_ptr()
                     {
                     return std::get<0>( impl_T );
                     }
                  T *impl_ptr() const
                     {
                     return std::get<0>( impl_T );
                     }
                  D &impl_del()
                     {
                     return std::get<1>( impl_T );
                     }
                  const D &impl_del() const
                     {
                     return std::get<1>( impl_T );
                     }

               private:
                  // if i were to use pair, i would be required to use
                  // [[no_unique_address]] this is obnoxious, so i use tuple
                  // which has that inbuilt.
                  std::tuple<T *, D> impl_T;
               };
         } // namespace base

   } // namespace scb
#endif