//
// Created by youh on 3/9/21.
//

#include "String.h"

std::ostream &operator<<( std::ostream &os, const String &s )
{
    os << "";
    for ( auto character : s )
        os << character;
    return os;
}
