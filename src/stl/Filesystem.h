#ifndef HW3_UTILITIES_H
#define HW3_UTILITIES_H

#endif //HW3_UTILITIES_H

#include <stdlib.h>
#include <fstream>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <iostream>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>

// TODO get rid of this - will have to add std:: to lots of places here...
using namespace std;

enum FileType {
   socket,
   symbolicLink,
   regularFile,
   blockDevice,
   directory,
   characterDevice,
   FIFO,
   unknown
};

char *ReadFile(char *filename, size_t &fileSize) {
   // Read the file into memory.
   // You'll soon learn a much more efficient way to do this.

   // Attempt to Create an istream and seek to the end
   // to get the size.
   ifstream ifs(filename, ios::ate | ios::binary);
   if (!ifs.is_open())
      return nullptr;
   fileSize = static_cast<size_t>(ifs.tellg());

   // Allocate a block of memory big enough to hold it.
   char *buffer = new char[fileSize];

   // Seek back to the beginning of the file, read it into
   // the buffer, then return the buffer.
   ifs.seekg(0);
   ifs.read(buffer, static_cast<streamsize>(fileSize));
   return buffer;
}

// potentially use enum
FileType getFiletype(mode_t mode) {
   switch (mode & S_IFMT) {
      case S_IFSOCK:
         return socket;
      case S_IFLNK:
         return symbolicLink;
      case S_IFREG:
         return regularFile;
      case S_IFBLK:
         return blockDevice;
      case S_IFDIR:
         return directory;
      case S_IFCHR:
         return characterDevice;
      case S_IFIFO:
         return FIFO;
      default:
         return unknown;
   }
}

bool DotName(const char *name) {
   return name[0] == '.' && (name[1] == 0 || (name[1] == '.' && name[2] == 0));
}

void ListDirectory(const string &filePath, vector <string> &files, const int maxDepth,
                   bool fileNameOnly = false) {
   DIR *handle = opendir(filePath.c_str());

   if (handle) {
      struct dirent *entry;
      entry = readdir(handle);
      while (entry) {
         struct stat statbuf;
         string childPath = filePath + '/';
         childPath += entry->d_name;
         if (stat(childPath.c_str(), &statbuf))
            cerr << "stat of " << filePath << " failed, errno = " << errno << endl;
         else {
            if (getFiletype(statbuf.st_mode) == regularFile) {
               files.push_back(fileNameOnly ? entry->d_name : childPath);
            }

            // potentially add file type info
            // Filetype(statbuf.st_mode)
            else if (getFiletype(statbuf.st_mode) == directory) {
               if (!DotName(entry->d_name) &&
                   (maxDepth > 0 || maxDepth == -1)) {
                  ListDirectory(childPath, files, maxDepth < 0 ? -1 : maxDepth - 1,
                                fileNameOnly);
               }
            }
            // potentially add size info
            // else cout << ", size = " << statbuf.st_size << endl;
         }
         entry = readdir(handle);
      }
      closedir(handle);
   } else cerr << "open handle failed, errno = " << errno;
}

size_t FileSize(int f) {
   struct stat fileInfo;
   fstat(f, &fileInfo);
   return fileInfo.st_size;
}

void mapfile(const char **file, char **start, char **end) {
   int f = open(*file, O_RDONLY);
   if (f != -1) {
      size_t fileSize = FileSize(f);
      *start = (char *) mmap(nullptr, fileSize, PROT_READ, MAP_PRIVATE, f, 0);
      if (start != MAP_FAILED) {
         *end = *start + fileSize;
      }
   }
}