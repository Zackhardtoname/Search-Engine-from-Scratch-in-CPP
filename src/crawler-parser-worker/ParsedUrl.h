//
// Created by youh on 3/9/21.
//

#ifndef CRAWLER_PARSEDURL_H
#define CRAWLER_PARSEDURL_H


class ParsedUrl {
public:
    const char *CompleteUrl;
    char *Service, *Host, *Port, *Path;

    explicit ParsedUrl(const char *url);

    ~ParsedUrl() {
        delete[] pathBuffer;
    }

private:
    char *pathBuffer;
};


#endif //CRAWLER_PARSEDURL_H
