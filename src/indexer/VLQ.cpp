//
// Created by youh on 3/11/21.
//

#include "VLQ.h"

String VLQ::Encode(uint64_t x) {
    String result;
    do {
        auto thisDigit = static_cast<uint8_t>(x & mask);
        // if not done yet
        if (x & ~mask)
            thisDigit |= ~mask;
        result.pushBack(thisDigit);

        x >>= 7;
    } while (x);

    return result;
}

uint64_t VLQ::Decode(const String &s) {
    unsigned shift = 0;
    uint64_t result = 0;
    for (uint8_t c: s) {
        result |= (static_cast<uint64_t>(c & mask) << shift);
        shift += 7;
    }
    return result;
}

// VLQ Stream Decoder

VLQStreamDecoder::VLQStreamDecoder(const char *s) {
    cursor = s;
}

uint64_t VLQStreamDecoder::next() {
    unsigned shift = 0;
    uint64_t result = 0;
    while(((uint8_t) *cursor >> 7 ) != 0) {
        result |= (static_cast<uint64_t>(*cursor & mask) << shift);
        shift += 7;
        ++cursor;
    }
    result |= (static_cast<uint64_t>(*cursor & mask) << shift);
    shift += 7;
    ++cursor;
    return result;
}

const char* VLQStreamDecoder::getCursor() const {
    return cursor;
}

void VLQStreamDecoder::setCursor(const char* cursor_in) {
    cursor = cursor_in;
}