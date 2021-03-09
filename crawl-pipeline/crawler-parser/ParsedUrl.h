//
// Created by youh on 3/2/21.
//

#ifndef SCOOBLE_SEARCH_PARSEDURL_H
#define SCOOBLE_SEARCH_PARSEDURL_H


class ParsedUrl {
public:
    const char *CompleteUrl;
    char *Service, *Host, *Port, *Path;

    ParsedUrl(const char *url);

    ~ParsedUrl();

private:
    char *pathBuffer;
};


#endif //SCOOBLE_SEARCH_PARSEDURL_H
