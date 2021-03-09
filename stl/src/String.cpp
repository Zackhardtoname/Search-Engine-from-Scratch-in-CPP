//
// Created by youh on 2/24/21.
//

#include "String.h"

std::ostream &operator<<( std::ostream &os, String &s )
{
    for ( char i : s )
        os << i;
    return os;
}
