#include "IndexChunk.h"

int main(int argc, char **argv) {
    IndexChunk chunk;

    chunk.urls.pushBack(String("https://example.com/?behavior=advertisement&password=password"));
    chunk.urls.pushBack(String("http://www.example.com/believe/bell"));
    chunk.urls.pushBack(String("https://example.com/birthday/actor.aspx"));
    chunk.urls.pushBack(String("https://mi.chaelwolf.com"));

    PostingList pl;
    pl.posts.pushBack(127);
    pl.posts.pushBack(128);
    pl.posts.pushBack(129);

    chunk.postingLists.pushBack(pl);

    PostingList pl2;
    pl2.posts.pushBack(127);
    pl2.posts.pushBack(128);
    pl2.posts.pushBack(129);

    chunk.postingLists.pushBack(pl2);

    chunk.writeToDisk();
}