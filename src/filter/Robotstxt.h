//
// Created by youh on 3/10/21.
//

#ifndef SCOOBLE_SEARCH_ROBOTSTXT_H
#define SCOOBLE_SEARCH_ROBOTSTXT_H

#include "../stl/String.h"
#include "../stl/Vector.h"

using scb::String;
using scb::Vector;

struct InvalidRobotstxtError : std::runtime_error {
    InvalidRobotstxtError() : std::runtime_error("invalid robots.txt") {};
};

/// A simplified robots.txt rule. It has four parts, all optional, and one
///   following another:
/// 1. Prefix string
/// 2. Wildcard (star symbol)
/// 3. Suffix after the wildcard
/// 4. End-of-string (dollar sign)
struct CrawlingRule {
    /// true if "Allowed", false if "Disallowed"
    bool Allowed;

    String Prefix;
    bool HasWildcard;
    String Suffix;
    bool HasEOS;

    CrawlingRule(const String &path, bool allowed);

    /// If this rule matches the given PATH.
    /// Do not pass in protocol, domain, and port.
    /// Do pass in the query part.
    /// Returns 0 if not matched, or the number of matched char.
    [[nodiscard]] int Match(const String &path) const;
};

/// A robots.txt wrapper class.
/// Use .Allow() to check if a path is allowed.
/// Use .Sitemap to retrieve the url of the sitemap if specified.
class Robotstxt {
    Vector<CrawlingRule> rules;

    void handleLine(const String &line);

public:
    String Sitemap;

    explicit Robotstxt(const String &content);

    /// Check if the given path is allowed by this robots.txt.
    /// Do not pass in protocol, domain, and port.
    /// Do pass in the query part.
    bool Allow(const String &path);
};


#endif //SCOOBLE_SEARCH_ROBOTSTXT_H
