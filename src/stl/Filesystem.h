#ifndef HW3_UTILITIES_H
#define HW3_UTILITIES_H

#include <dirent.h>
#include <fcntl.h>
#include <fstream>
#include <iostream>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
//#include <vector>
#include "../stl/RAII.h"
#include "../stl/utils.h"

enum FileType
{
   Socket,
   SymbolicLink,
   RegularFile,
   BlockDevice,
   Directory,
   CharacterDevice,
   FIFO,
   Unknown
};

char *ReadFile( char *filename, size_t &fileSize )
   {
   // Read the file into memory.
   // You'll soon learn a much more efficient way to do this.

   // Attempt to Create an istream and seek to the end
   // to get the size.
   std::ifstream ifs( filename, std::ios::ate | std::ios::binary );
   if ( !ifs.is_open() )
      return nullptr;
   fileSize = static_cast<size_t>( ifs.tellg() );

   // Allocate a block of memory big enough to hold it.
   char *buffer = new char[ fileSize ];

   // Seek back to the beginning of the file, read it into
   // the buffer, then return the buffer.
   ifs.seekg( 0 );
   ifs.read( buffer, static_cast<std::streamsize>( fileSize ) );
   return buffer;
   }

// potentially use enum
FileType getFiletype( mode_t mode )
   {
   switch ( mode & S_IFMT )
      {
      case S_IFSOCK:
         return Socket;
      case S_IFLNK:
         return SymbolicLink;
      case S_IFREG:
         return RegularFile;
      case S_IFBLK:
         return BlockDevice;
      case S_IFDIR:
         return Directory;
      case S_IFCHR:
         return CharacterDevice;
      case S_IFIFO:
         return FIFO;
      default:
         return Unknown;
      }
   }

bool DotName( const char *name )
   {
   return name[ 0 ] == '.' &&
          ( name[ 1 ] == 0 || ( name[ 1 ] == '.' && name[ 2 ] == 0 ) );
   }

void ListDirectory( scb::String &filePath, scb::Vector<scb::String> &files,
                    const int maxDepth, bool fileNameOnly = false )
   {
   DIR *handle = opendir( filePath.cstr() );

   if ( handle )
      {
      struct dirent *entry;
      entry = readdir( handle );
      while ( entry )
         {
         struct stat statbuf;
         scb::String childPath = filePath + scb::String( "/" );
         childPath += entry->d_name;
         if ( stat( childPath.cstr(), &statbuf ) )
            std::cerr << "stat of " << filePath.cstr() << " failed, errno = " << errno
                      << std::endl;
         else
            {
            if ( getFiletype( statbuf.st_mode ) == RegularFile )
               {
               files.pushBack( fileNameOnly ? entry->d_name : childPath );
               }

            // potentially add file type info
            // Filetype(statbuf.st_mode)
            else if ( getFiletype( statbuf.st_mode ) == Directory )
               {
               if ( !DotName( entry->d_name ) &&
                    ( maxDepth > 0 || maxDepth == -1 ) )
                  {
                  ListDirectory( childPath, files,
                                 maxDepth < 0 ? -1 : maxDepth - 1,
                                 fileNameOnly );
                  }
               }
            // potentially add size info
            // else cout << ", size = " << statbuf.st_size << endl;
            }
         entry = readdir( handle );
         }
      closedir( handle );
      }
   else
      std::cerr << "open handle failed, errno = " << errno;
   }

//size_t FileSize( int f )
//   {
//   struct stat fileInfo;
//   fstat( f, &fileInfo );
//   return fileInfo.st_size;
//   }

int mapfile( const char **file, char **start, char **end )
   {
   int f = open( *file, O_RDONLY );
   if ( f != -1 )
      {
      size_t fileSize = FileSize( f );
      *start = (char *)mmap( nullptr, fileSize, PROT_READ, MAP_PRIVATE, f, 0 );
      if ( start != MAP_FAILED )
         {
         *end = *start + fileSize;
         }
      }
   return f;
   }

#endif // HW3_UTILITIES_H