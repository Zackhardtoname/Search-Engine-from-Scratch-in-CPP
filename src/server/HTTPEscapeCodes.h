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

      codeToCharTable->Optimize();
   }

   ~URLDecoder() {
      delete codeToCharTable;
   }

   // TODO func here
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
            escapedOut.pushBack(codeToCharTable->find(characterCode)->value);
         }
         else
         {
            escapedOut.pushBack(*character);
         }
      }

      return escapedOut;
   }
};