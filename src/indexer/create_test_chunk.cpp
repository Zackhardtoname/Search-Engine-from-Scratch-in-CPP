#include "IndexChunk.h"

int main( int argc, char **argv )
   {
   IndexChunk chunk;

   chunk.urls.pushBack( String(
      "https://example.com/?behavior=advertisement&password=password" ) );
   chunk.urls.pushBack( String( "http://www.example.com/believe/bell" ) );
   chunk.urls.pushBack( String( "https://example.com/birthday/actor.aspx" ) );
   chunk.urls.pushBack( String( "https://mi.chaelwolf.com" ) );

   PostingList pl;
   pl.posts.pushBack( 127 );
   pl.posts.pushBack( 128 );
   pl.posts.pushBack( 129 );

   chunk.postingLists.pushBack( pl );

   PostingList pl2;
   pl2.posts.pushBack( 689629595 );
   pl2.posts.pushBack( 413866503 );
   pl2.posts.pushBack( 780514417 );
   pl2.posts.pushBack( 327549560 );
   pl2.posts.pushBack( 1 );
   pl2.posts.pushBack( 2 );
   pl2.posts.pushBack( 3 );
   pl2.posts.pushBack( 909477693 );
   pl2.posts.pushBack( 561908299 );
   pl2.posts.pushBack( 938566262 );
   pl2.posts.pushBack( 883589742 );

   chunk.postingLists.pushBack( pl2 );

   chunk.writeToDisk( "test.chunk" );
   }