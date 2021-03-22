//
// Created by youh on 3/11/21.
//

#ifndef SCOOBLE_SEARCH_VLQ_H
#define SCOOBLE_SEARCH_VLQ_H


#include <cstdint>
#include "../stl/String.h"

using scb::String;

/// VLQ encoding (https://en.wikipedia.org/wiki/Variable-length_quantity) implementation.
/// We are small-endian, i.e. s[0] is the LSB.
class VLQ {
    constexpr static uint8_t mask = 0x7f; // 0111 1111
public:
    static String Encode(uint64_t);
    static uint64_t Decode(const String& s);
};

class VLQStreamDecoder {
private:
    const char *cursor;
    constexpr static uint8_t mask = 0x7f; // 0111 1111
public:
    VLQStreamDecoder(const char *s);
    uint64_t Next();
    const char* getCursor() const;
    void setCursor(const char* cursor_in);
};

#endif //SCOOBLE_SEARCH_VLQ_H
