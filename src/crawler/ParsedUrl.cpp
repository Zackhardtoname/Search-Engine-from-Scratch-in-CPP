//
// Created by youh on 3/9/21.
//

#include <cstring>
#include "ParsedUrl.h"

ParsedUrl::ParsedUrl(const String &url) {
    // anything after pound is meaningless as page URL
    int pound = url.find("#");
    if (pound == String::npos)
        CompleteUrl = url;
    else
        CompleteUrl = url.substr(0, pound);

    // Service
    int i = 0;
    for (; i < CompleteUrl.size() && CompleteUrl[i] != ':'; i++);
    Service = CompleteUrl.substr(0, i);

    // Host
    i += 3;
    int hostStart = i;
    for (; i < CompleteUrl.size() && CompleteUrl[i] != ':' && CompleteUrl[i] != '/'; i++);
    Host = CompleteUrl.substr(hostStart, i - hostStart);

    // Port
    if (i < CompleteUrl.size() && CompleteUrl[i] == ':') {
        int portStart = i + 1;
        for (; i < CompleteUrl.size() && CompleteUrl[i] != '/'; i++);
        Port = CompleteUrl.substr(portStart, i - portStart);
    }

    HostPort = CompleteUrl.substr(hostStart, i - hostStart);

    // Path
    hostStart = i;
    for (; i < CompleteUrl.size() && CompleteUrl[i] != '?'; i++);
    Path = CompleteUrl.substr(hostStart, i - hostStart);
}

String ParsedUrl::Normalize(const String &path) {
    // edge case
    if (path.empty())
        return CompleteUrl;
    // if fully qualified
    if (path.hasPrefix("http://") || path.hasPrefix("https://"))
        return path;

    // if absolute path
    if (path.front() == '/')
        return Service + "://" + HostPort + path;

    // relative path
    String result = Service + "://" + HostPort + Path;
    if (result.back() != '/')
        result.pushBack('/');
    result += path;

    return ParsedUrl(result).CompleteUrl;
}
