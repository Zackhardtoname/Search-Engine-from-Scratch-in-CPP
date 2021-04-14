cmake_minimum_required( VERSION 3.10 ) project( read - chunk )

   set( CMAKE_CXX_STANDARD 17 )

      add_executable( read - chunk ReadChunk.cpp../../ stl / String.cpp../../
                                indexer / VLQ.cpp )

         add_executable( generate - chunk GenerateChunk.cpp../../ indexer /
                                       VLQ.cpp../../ indexer /
                                       IndexChunk.cpp../../ stl /
                                       utils.cpp../../ stl / String.cpp )