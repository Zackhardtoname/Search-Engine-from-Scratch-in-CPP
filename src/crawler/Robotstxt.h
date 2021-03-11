//
// Created by youh on 3/10/21.
//

#ifndef SCOOBLE_SEARCH_ROBOTSTXT_H
#define SCOOBLE_SEARCH_ROBOTSTXT_H

#include "String.h"

using scb::String;

struct InvalidRobotstxtError : std::runtime_error {
    InvalidRobotstxtError() : std::runtime_error("invalid robots.txt") {};
};

class Robotstxt {
    void handleLine(const String &line);

public:
    Robotstxt(const String &content);
};


#endif //SCOOBLE_SEARCH_ROBOTSTXT_H
