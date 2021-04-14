#ifndef CONSTRAINT_SOLVER_H
#define CONSTRAINT_SOLVER_H

/*
 * Constraint Solver
 *
 * Provides static methods for getting matches
 *
 * Created by mjwolf on 4/12/2021
 */

#include "../indexer/IndexChunk.h"
#include "ConstraintSolver.h"
#include "ISR/ISR.h"

#include <cstdio>
#include <cstring>
#include <dirent.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>

#define TestFolder "testIndex"
#define IndexChunkExtension "chunk"

using std::cerr;
using std::cout;
using std::endl;

size_t fileSize( int f )
   {
   struct stat fileInfo;
   fstat( f, &fileInfo );
   return fileInfo.st_size;
   }

class ConstraintSolver
   {
   private:
      /*
      static void matchesFromIndex( AST *ast, DocumentMatch matches[],
                                    unsigned matchLimit,
                                    const scb::String &path )
         {

         // Open chunk file for reading
         int fd = open( path.cstr(), O_RDONLY );
         if ( fd == -1 )
            {
            cerr << "Error opening file " << path << endl;
            exit( 1 );
            }

         // Map in chunk, initially cast as header
         auto *chunk = reinterpret_cast<ContiguousChunk *>(
            mmap( nullptr, fileSize( fd ), PROT_READ, MAP_PRIVATE, fd, 0 ) );
         if ( chunk == MAP_FAILED )
            {
            cerr << "Error mmaping file " << path << endl;
            exit( 1 );
            }

         cout << "Reading '" << path.cstr() << "'\n";

         // * Read header
         cout << "\tDocument count: " << chunk->getHeader()->DocumentCount
              << endl;
         cout << "\tDocument Table Start: "
              << chunk->getHeader()->DocumentTableOffset << endl;
         cout << "\tDictionary Offset: "
              << chunk->getHeader()->WordToPostingListDictionaryOffset << endl;
         cout << "\tPosting List Offset: "
              << chunk->getHeader()->PostingListsOffset << endl;

         // Get matches

         scb::String key =
            scb::String( static_cast<LiteralNode *>( ast )->num.c_str() );
         WordISR isr = chunk->getWordISR( key );

         unsigned i;

         while ( i = isr.Next() )
            {
            std::cout << i << std::endl;
            }
         }
         */

   public:
      // TODO pass in folder to get matches from
      /*
      static void GetMatches( AST *ast, DocumentMatch matches[],
                              unsigned matchLimit )
         {

         // For each index file in test folder
         DIR *fd;
         char *filename, *ext;
         int ret;
         struct dirent *dir;
         fd = opendir( TestFolder ); // specify the directory name
         if ( fd )
            {
            while ( ( dir = readdir( fd ) ) != NULL )
               {
               filename = strtok( dir->d_name, "." );
               ext = strtok( NULL, "." );

               if ( ext != NULL )
                  {
                  ret = strcmp( ext, IndexChunkExtension );
                  if ( ret == 0 )
                     {
                     scb::String filepath( TestFolder );
                     filepath += "/";
                     filepath += filename;
                     filepath += ".";
                     filepath += ext;

                     matchesFromIndex( ast, matches, matchLimit, filepath );
                     }
                  }
               }
            closedir( fd );
            }

         // For each .index file:
         // run the root ISR
         }
         */
   };

#endif // CONSTRAINT_SOLVER_H