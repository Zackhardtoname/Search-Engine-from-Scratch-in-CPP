#include "../stl/String.h"
#include "../stl/StringHash.h"
#include "../stl/HashTable.h"

using scb::String;

class URLDecoder {
private:
   scb::HashTable<const String, char, scb::StringHash> *codeToCharTable;

public:

   URLDecoder() {
      codeToCharTable = new scb::HashTable<const String, char, scb::StringHash>();

      codeToCharTable->insert(String("20"), ' ');
      codeToCharTable->insert(String("26"), '&');
      codeToCharTable->insert(String("7C"), '|');
      codeToCharTable->insert(String("40"), '@');
      codeToCharTable->insert(String("25"), '%');
      // TODO: add more symbol codes if necessary... if an unrecognized code is found,
      // we currently just swap it for a space

      codeToCharTable->Optimize();
   }

   ~URLDecoder() {
      delete codeToCharTable;
   }

   String escapeCodesToString(String &escapedIn) {

      String escapedOut;
      for(auto character = escapedIn.begin(); character != escapedIn.end(); character++ )
      {
         if(*character == '%')
         {
            // Decode
            String characterCode; 
            characterCode += *(++character);
            characterCode += *(++character);
            auto it = codeToCharTable->find(characterCode);
            if (it != codeToCharTable->end())
               escapedOut.pushBack(codeToCharTable->find(characterCode)->value);
            else
               escapedOut.pushBack(' ');
         }
         else
         {
            escapedOut.pushBack(*character);
         }
      }

      return escapedOut;
   }
};