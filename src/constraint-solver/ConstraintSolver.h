#ifndef CONSTRAINT_SOLVER_H
#define CONSTRAINT_SOLVER_H

#include "../query_parser/query_parser.h"
#include "../indexer/IndexChunk.h"
#include "QueryListener.h"

#include <dirent.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>

#define TestFolder "testIndex"
#define IndexChunkExtension "chunk"

using std::cout;
using std::cerr;
using std::endl;

size_t fileSize( int f )
   {
   struct stat fileInfo;
   fstat( f, &fileInfo );
   return fileInfo.st_size;
   }


class ConstraintSolver {
private:
    static void matchesFromIndex(AST * ast, DocumentMatch matches[], unsigned matchLimit, scb::String path) {

        // Open chunk file for reading
        int f = open( path.cstr(), O_RDONLY );
        if ( f == -1 )
            {
            cerr << "Error opening file " << path.cstr() << endl;
            exit(1);
            }

        // Map in chunk, initially cast as header
        ChunkHeader *header = reinterpret_cast<ChunkHeader*>(mmap( nullptr, fileSize( f ), PROT_READ, MAP_PRIVATE, f, 0 ));
        
        cout << "Reading '" << path.cstr() << "'\n";

        // * Read header 
        cout << "\tDocument count: " << header->DocumentCount << endl;
        cout << "\tDocument Table Start: " << header->DocumentTableOffset << endl;
        cout << "\tDictionary Offset: " << header->WordToPostingListDictionaryOffset
                << endl;
        cout << "\tPosting List Offset: " << header->PostingListsOffset << endl;

        // Get matches
        
    }


public:

    // TODO pass in folder to get matches from
    static void GetMatches(AST * ast, DocumentMatch matches[], unsigned matchLimit) {
        
        // For each index file in test folder
        DIR *fd;
        char *filename,*ext;
        int ret;
        struct dirent *dir;
        fd = opendir(TestFolder); //specify the directory name
        if (fd)
        {
            while ((dir = readdir(fd)) != NULL)
            {
                filename = strtok(dir->d_name,".");
                ext = strtok(NULL,".");

                if(ext!=NULL)
                {
                    ret=strcmp(ext,IndexChunkExtension);
                    if(ret==0)
                    {
                        scb::String filepath(TestFolder);
                        filepath += "/";
                        filepath += filename;
                        filepath += ".";
                        filepath += ext;

                        matchesFromIndex(ast, matches, matchLimit, filepath);
                    }
                }
            }
            closedir(fd);
        }

        // For each .index file:
            // run the root ISR
    }
};

#endif // CONSTRAINT_SOLVER_H