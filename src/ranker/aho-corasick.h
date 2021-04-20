#include <algorithm>
#include <cstring>
#include <iostream>
#include <set>
#include <string>

const int ALPHABET_LIMIT = 256;
const int UNUSED_CHARACTER = 255;

class Trie
   {
   private:
      struct Node
         {
            Node *son[ ALPHABET_LIMIT ];
            Node *go[ ALPHABET_LIMIT ];
            Node *parent;
            Node *suffLink;
            Node *up;
            size_t charToParent;
            bool isLeaf;
            std::set<int> patternNumbers;

            Node()
               {
               parent = NULL;
               suffLink = NULL;
               up = NULL;
               charToParent = UNUSED_CHARACTER;
               isLeaf = false;
               memset( son, (char)NULL, sizeof son );
               memset( go, (char)NULL, sizeof go );
               }
         };

      Node *root;
      int nodeCount;

      Node *getSuffLink( Node *v )
         {
         if ( v->suffLink == NULL )
            {
            if ( v == this->root || v->parent == this->root )
               {
               v->suffLink = this->root;
               }
            else
               {
               v->suffLink = getGo( getSuffLink( v->parent ), v->charToParent );
               }
            }
         return v->suffLink;
         }
      Node *getGo( Node *v, size_t c )
         {
         if ( v->go[ c ] == NULL )
            {
            if ( v->son[ c ] != NULL )
               {
               v->go[ c ] = v->son[ c ];
               }
            else if ( v == this->root )
               {
               v->go[ c ] = this->root;
               }
            else
               {
               v->go[ c ] = getGo( getSuffLink( v ), c );
               }
            }
         return v->go[ c ];
         }
      Node *getUp( Node *v )
         {
         if ( v->up == NULL )
            {
            Node *suffLink = getSuffLink( v );
            if ( suffLink->isLeaf )
               {
               v->up = suffLink;
               }
            else if ( suffLink == this->root )
               {
               v->up = this->root;
               }
            else
               {
               v->up = getUp( suffLink );
               }
            }

         return v->up;
         }
      void deleteNode( Node *v )
         {
         for ( int i = 0;  i < ALPHABET_LIMIT;  ++i )
            {
            if ( v->son[ i ] != NULL )
               {
               deleteNode( v->son[ i ] );
               }
            }

         delete v;
         }
      inline size_t charInCharset( char c )
         {
         return 128 + ( (int)c );
         }

   public:
      Trie()
         {
         this->root = new Trie::Node();
         this->nodeCount = 1;
         }
      ~Trie()
         {
         deleteNode( this->root );
         }

      void addPattern( const std::string &s, int n )
         {
         Trie::Node *cur = this->root;
         for ( int i = 0;  i < s.length();  ++i )
            {
            size_t c = charInCharset( s[ i ] );
            if ( cur->son[ c ] == NULL )
               {
               cur->son[ c ] = new Trie::Node();
               cur->son[ c ]->parent = cur;
               cur->son[ c ]->charToParent = c;
               }
            cur = cur->son[ c ];
            }
         cur->isLeaf = true;
         cur->patternNumbers.insert( n );
         }
      void processText( const std::string &t, std::set<int> &found )
         {
         std::set<Trie::Node *> visitedUpLinks;
         found.clear();
         Trie::Node *cur = this->root;
         for ( int i = 0;  i < t.length();  ++i )
            {
            size_t c = 128 + ( (int)t[ i ] );
            cur = getGo( cur, c );

            Trie::Node *collector = cur;
            do
               {
               if ( visitedUpLinks.find( collector ) != visitedUpLinks.end() )
                  {
                  break;
                  }
               for ( std::set<int>::iterator it =
                        collector->patternNumbers.begin();
                     it != collector->patternNumbers.end();  ++it )
                  {
                  found.insert( *it );
                  }
               visitedUpLinks.insert( collector );

               collector = getUp( collector );
               }
            while ( collector != this->root );
            }
         }

      int getNodeCount()
         {
         return this->nodeCount;
         }
   };
