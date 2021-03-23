//
// Created by youh on 3/22/21.
//

#include <cassert>
#include <cstdint>
#include <cstring>
#include <cstdlib>
#include <fcntl.h>
#include <iostream>
#include <unistd.h>
#include <cmath>

int main(int argc, char **argv) {
   if (argc != 4) {
      std::cout << "usage: " << argv[0] << " [n] [prob] [bloom file]" << std::endl;
      return 1;
   }

   long n;
   sscanf(argv[1], "%ld", &n);
   double prob;
   sscanf(argv[2], "%lf", &prob);

   uint64_t numSlots = long(n * abs(log(prob)) / (log(2) * log(2)));
   uint32_t numHash = int(numSlots * log(2) / n);

   std::cout << "using n=" << n << ", prob=" << prob << '\n'
             << "we will use numHash=" << numHash << '\n'
             << "file size=" << (numSlots / 8 / 1024) << "KB" << '\n'
             << "sounds good? (y)" << std::endl;

   char ans = '\0';
   while (ans != 'y')
      std::cin >> ans;

   int fd = open(argv[3], O_CREAT | O_RDWR | O_APPEND, 0644);
   assert(fd != -1 && "open() failed");
   assert(write(fd, &numHash, sizeof(uint32_t)) == sizeof(uint32_t) && "write() failed");
   assert(ftruncate(fd, sizeof(uint32_t) + numSlots / 8) != -1 && "ftruncate() failed");
}
