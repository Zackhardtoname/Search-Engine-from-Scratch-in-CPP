//
// Created by youh on 3/9/21.
//


#include <cassert>
#include "ParsedHttpResponse.h"

ParsedHttpResponse::ParsedHttpResponse(const String &resp) {
    int codeStart = resp.find(" ");
    if (codeStart == String::npos || (++codeStart) >= resp.length())
        throw HttpParserError();

    int codeEnd = resp.find(" ", codeStart);
    if (codeEnd == String::npos)
        throw HttpParserError();

    StatusCode = resp.substr(codeStart, codeEnd - codeStart);

    int headerStart = resp.find("\r\n", codeEnd);
    if (headerStart == String::npos)
        throw HttpParserError();

    // ending of all headers
    int headerEnd = resp.find("\r\n\r\n", headerStart);
    if (headerEnd == String::npos)
        throw HttpParserError();

    // parse the interesting headers
    bool chunked = false;
    while (headerStart != headerEnd) {
        headerStart += 2; // skip "\r\n"
        int lineEnd = resp.find("\r\n", headerStart);
        assert(lineEnd != String::npos);

        // now, [headerStart, lineEnd) contains one header, without the \r\n part
        String headerLine = resp.substr(headerStart, lineEnd - headerStart);
        if (headerLine.hasPrefix(CONTENT_LANGUAGE_HEADER)) {
            Language = parseHeaderValue(headerLine, CONTENT_LANGUAGE_HEADER);
        } else if (headerLine.hasPrefix(LOCATION_HEADER)) {
            Location = parseHeaderValue(headerLine, LOCATION_HEADER);
        } else if (headerLine.hasPrefix(TRANSFER_ENCODING_HEADER)) {
            String encoding = parseHeaderValue(headerLine, TRANSFER_ENCODING_HEADER);
            if (encoding != "identity" && encoding != "chunked")
                throw HttpParserError("encoding not supported");
            if (encoding == "chunked")
                chunked = true;
        } else if (headerLine.hasPrefix(CONTENT_TYPE_HEADER)) {
            ContentType = parseHeaderValue(headerLine, CONTENT_TYPE_HEADER);
        }

        headerStart = lineEnd;
    }

    // read body
    int bodyStart = headerEnd + 4;
    if (!chunked) {
        Body = resp.substr(bodyStart);
        return;
    }

    // read chunked body
    while (bodyStart < resp.length()) {
        // get the chunk length str
        int lenEnd = resp.find("\r\n", bodyStart);
        // got the terminal chunk
        if (lenEnd == bodyStart)
            break;
        if (lenEnd == String::npos)
            throw HttpParserError("malformed chunked body");
        String lenStr = resp.substr(bodyStart, lenEnd - bodyStart);

        // parse the chunk length str
        uint chunkLen;
        try {
            chunkLen = std::stoul(lenStr.cstr(), nullptr, 16);
        } catch (std::invalid_argument &) {
            throw HttpParserError("malformed chunk length");
        }

        // Another common way to signal terminal chunk
        if(chunkLen == 0)
            break;

        int chunkStart = lenEnd + 2;
        if (chunkStart + chunkLen >= resp.length())
            throw HttpParserError("malformed chunked body");

        Body += resp.substr(chunkStart, chunkLen);
        bodyStart = chunkStart + chunkLen + 2; // +2 because chunk ends with \r\n as well
    }
}

String ParsedHttpResponse::parseHeaderValue(const String &headerLine, const String &key) {
    int valStart = key.length();
    for (; valStart < headerLine.size() && headerLine[valStart] == ' '; valStart++);
    int valEnd = valStart;
    for (; valEnd < headerLine.size() && headerLine[valEnd] != ' '; valEnd++);
    return headerLine.substr(valStart, valEnd - valStart);
}
