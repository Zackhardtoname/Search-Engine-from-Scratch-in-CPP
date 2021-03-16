//
// Created by youh on 3/9/21.
//

#include <cstring>
#include "ParsedUrl.h"

ParsedUrl::ParsedUrl(const String &url) {
    CompleteUrl = url;

    // Service
    int i = 0;
    for (; i < url.size() && url[i] != ':'; i++);
    Service = url.substr(0, i);

    // Host
    i += 3;
    int hostStart = i;
    for (; i < url.size() && url[i] != ':' && url[i] != '/'; i++);
    Host = url.substr(hostStart, i - hostStart);

    // Port
    if (i < url.size() && url[i] == ':') {
        int portStart = i + 1;
        for (; i < url.size() && url[i] != '/'; i++);
        Port = url.substr(portStart, i - portStart);
    }

    HostPort = url.substr(hostStart, i - hostStart);

    // Path
    hostStart = i;
    for (; i < url.size() && url[i] != '?'; i++);
    Path = url.substr(hostStart, i - hostStart);
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
    return result;
}
