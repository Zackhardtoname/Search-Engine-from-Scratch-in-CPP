# Index Chunks

Index chunks are stored in `.chunk` files.

You can view them simply using `hexdump -C <.chunk file>`, or view the interpretation of their contents using the `readChunks` tool in `/src/utils`.

# Chunk File Structure

Each chunk file consists of a header section and a data section. The data in the header section is used to access divisions of the data section.

As of March 20, 2021, the chunk file structure is:

- A 16B header, consisting of:
    - `i32` Document count
    - `i32` Offset of Dictionary
    - `i32` Offset of Posting Lists section
    - `i32` Offset of end of data
- A data section, taking up `INDEX_CHUNK_TARGET_SIZE - sizeof(header)` bytes
    - The list of urls, as character arrays, separated by a single null terminator
    - The dictionary, stored as a hash blob. Maps a word to its posting list's offset
    - An array of posting lists, each of which consists of:
        - A 4B header, consisting of:
            - `i32` offset of first post
        - A variable-length string of variable-bit-encoded integers
        - A null terminator, signifying the end of the posting list

Note that the null terminator that closes each posting list is equivalent to the variable-length encoding of a `0`. This will not occur as a normal delta, so it is safe to use as a terminator.

At the moment, posting lists do not contain synchronization tables. Because of this, the offset of the first posting list will always be 0.