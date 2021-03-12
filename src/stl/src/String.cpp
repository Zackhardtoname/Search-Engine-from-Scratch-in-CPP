//
// Created by youh on 3/9/21.
//

#include "../include/String.h"

std::ostream &scb::operator<<( std::ostream &os, const scb::String &s )
{
    os << "";
    for ( auto character : s )
        os << character;
    return os;
}
