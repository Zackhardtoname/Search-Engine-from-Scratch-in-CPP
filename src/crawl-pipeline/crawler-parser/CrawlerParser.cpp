//
// Created by youh on 3/2/21.
//

#include <mqueue.h>

#include "CrawlerParserManager.h"
#include "CrawlerParser.h"
#include "defs.h"
#include "String.h"

void CrawlerParser::RunForever() {
    char msgBuf[URL_MAX_LENGTH];
    while (true) {
        ssize_t msgSize = mq_receive(urlMQ, msgBuf, URL_MAX_LENGTH, nullptr);
        String url(msgBuf, msgBuf + msgSize);
        std::cout << "Got a URL: " << url << std::endl;

        FetchResult fetchResult = fetch(url);
        String &response = fetchResult.RespBody;

        if (!response.empty()) {
            HtmlParser parser(response.cstr(), response.size());
            // TODO send parsed results to some queue
            std::cout << parser.base << std::endl;
            for (const auto &word : parser.words) {
                std::cout << word << std::endl;
            }
        }
    }
}

FetchResult CrawlerParser::fetch(const String &url) {
    // TODO refactor robots.txt to elsewhere
    const std::string headerContentDiv = "\r\n\r\n";
    std::string robotUrlString = std::string(urlString.cstr()) + "/robots.txt";
    ParsedUrl robotUrl(robotUrlString.c_str());
    std::string robotResponse = getHtmlResponseWithSockets(&robotUrl);
    std::transform(robotResponse.begin(), robotResponse.end(), robotResponse.begin(),
                   [](unsigned char c) { return std::tolower(c); });

    // This should still work if there is no robot.txt
    auto disallowStart = robotResponse.find("user-agent: *");
    auto nextDisallowStart = robotResponse.find("user-agent:", disallowStart + 1);
    auto disallowEnd = robotResponse.find("disallow: /\n", disallowStart + 1);
    if (disallowEnd < nextDisallowStart) {
        // TODO handle it properly (potentially design custom response code as enum)
        exit(1);
    }

    ParsedUrl parsedUrl(url.cstr());

    String response = getHtmlResponseWithSockets(&url);
    int headerEnd = response.find("\r\n\r\n");

    // Check if the requested Url returned with a page redirect, and if so
    // recursively redirect to the updated page listed under the 'Location'
    // field.

    // TODO Handle exception (malformed header)
    int statusCode = stoi(response.substr(9, 11));
    std::string language = "";
    std::string contentLanguageHeader = "Content-Language: ";
    auto languageStart = response.find(contentLanguageHeader);
    if (languageStart != std::string::npos) {
        languageStart += contentLanguageHeader.size();
        auto languageEnd = response.find("\r\n", languageStart, 2);
        language = response.substr(languageStart, languageEnd - languageStart);
    }

    std::cout << statusCode << language << std::endl;

    while (response.find("HTTP/1.1 301 Moved Permanently\r\n") < headerEnd) {
        std::string key = "Location: ";
        auto beginNewUrl = response.find(key);
        auto endNewUrl = response.find("\r\n", beginNewUrl, 2);
        std::string newUrl(response.begin() + beginNewUrl + key.size(),
                           response.begin() + endNewUrl);
        std::cout << "301: " << newUrl << std::endl;
        if (newUrl.at(0) == '/') {
            newUrl = std::string(urlCStr) + newUrl;
        }

        ParsedUrl redirectUrl(newUrl.c_str());
        url = redirectUrl;
        response =
                getHtmlResponseWithSockets(&url);
        headerEnd = response.find(headerContentDiv);
    }

    // Remove the response header from the printed output in order to print
    // only the page HTML
    auto chunked = response.find("chunked");
    // TODO parse the resonse here as well
    if (chunked < headerEnd) {
        // Check if the Transfer-Encoding field includes the 'chunked' value by
        // checking if the keyword 'chunked' comes before the HTML headerEnd. If
        // so, remove the next line from the output since it contains the chunk
        // quantity and isn't real HTML.
        std::string htmlResponse(response.begin() + headerEnd + 4,
                                 response.end());

        auto startHtml = htmlResponse.find("\n");
        auto endHtml = htmlResponse.rfind("</html>");
        htmlResponse.assign(htmlResponse.begin() + startHtml + 1,
                            htmlResponse.begin() + endHtml + 7);
        response = htmlResponse;
    }

    FetchResult fetchResult(url, statusCode, language, response.substr(0, headerEnd),
                            response.substr(headerEnd + headerContentDiv.size()));
    return fetchResult;
}

FetchResult::FetchResult(const ParsedUrl &url, int statusCode, String lang, String respHeader,
                         String respBody)
        : url(url), StatusCode(statusCode),
          Lang(std::move(lang)), RespHeader(std::move(respHeader)),
          RespBody(std::move(respBody)) {}
