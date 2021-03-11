//
// Created by youh on 3/10/21.
//

#include "Robotstxt.h"

Robotstxt::Robotstxt(const String &content) {
    int starSectionPos = content.find("User-agent: *");
    // tolerate no space after semicolon.
    if (starSectionPos == String::npos)
        starSectionPos = content.find("User-agent:*");
    if (starSectionPos == String::npos)
        throw InvalidRobotstxtError();

    int nextSection = content.find("User-agent:", starSectionPos);
    String starSection = content.substr(starSectionPos, nextSection - starSectionPos);
    starSection.pushBack('\n'); // to guarantee that we handle the last line

    String line;
    for (char c: starSection) {
        if (c == '\r' || c == '\n') {
            // ignore multiple consecutive \r\n
            if (line.empty())
                continue;
            handleLine(line);
            line = "";
        } else
            line.pushBack(c);
    }
}

void Robotstxt::handleLine(const String &line) {
    if (line.hasPrefix("Allow:")) {
        // TODO
    } else if (line.hasPrefix("Disallow:")) {
        // TODO
    }
    // ignore other stuffs
}
