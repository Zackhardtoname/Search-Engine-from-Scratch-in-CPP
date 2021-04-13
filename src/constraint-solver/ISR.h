#ifndef ISR_H
#define ISR_H

/*
 * Index Stream Readers
 * 
 * Interface for a general ISR and implementations for specific types of ISRs.
 * 
 * ISRs are an abstraction used to search each index for matches.
 * 
 * Created by mjwolf on 4/12/2021
 */

#include "../indexer/VLQ.h"

class ISR {
    virtual unsigned Next() = 0;
};

class WordISR : public ISR {
private:
    const char* cursor;
    VLQStreamDecoder *decoder;

public:

    WordISR(const char* cursor_in) : cursor(cursor_in) { 
        decoder = new VLQStreamDecoder(cursor);
    }

    ~WordISR() {
        delete decoder;
    }

    /*
     * Returns 0 if end of stream or if key is not in index
     */
    unsigned Next() {
        if(cursor == nullptr) {
            return 0;
        }

        return decoder->Next();
    }

    // unsigned GetDocumetnCount( );
    // unsigned GetNumberOfOccurences( );
    // virtual Post *GetCurrentPost( );
};

#endif //ISR_H