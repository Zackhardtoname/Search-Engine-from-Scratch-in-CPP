//
// Created by youh on 3/20/21.
//

#ifndef SCOOBLE_SEARCH_PAIR_H
#define SCOOBLE_SEARCH_PAIR_H

namespace scb {
   template<typename A, typename B>
   struct Pair {
      A first;
      B second;

      template<typename F, typename S>
      Pair(F &&first, S &&second): first(std::forward<F>(first)), second(std::forward<S>(second)) {
      }
   };

   template<typename A, typename B>
   Pair<A, B> MakePair(A &&first, B &&second) {
      return Pair(first, second);
   }
}

#endif //SCOOBLE_SEARCH_PAIR_H
