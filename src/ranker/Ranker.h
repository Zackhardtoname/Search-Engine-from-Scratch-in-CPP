#include <bits/stdc++.h>

// "harry potter and the goblet of fire"--initial test

// harry * potter * 

// best link types:
// .gov -> .edu -> .com -> .net -> .others
//std::unordered_map<std::string, size_t> RANKED_LINKS;
struct LinkInfo {
    std::string link;
    int domainType;
    size_t length;
    size_t numSlashes;
    // double factor; maybe we need this?
};

auto staticRankComparator(const LinkInfo &linkA, const LinkInfo &linkB) -> bool {
        if (linkA.domainType == linkB.domainType) {
            if (linkA.length == linkB.length) {
                if (linkA.numSlashes == linkB.numSlashes){
                    return linkA.link < linkB.link;
                }
                return linkA.numSlashes < linkB.numSlashes;
            }
            return linkA.length < linkB.length ;
        }
        return linkA.domainType > linkB.domainType;
}

std::vector<LinkInfo> staticRank(std::vector<std::string> urls){
    size_t numLinks = urls.size();
    float linkFactor = 1/numLinks;
    std::vector<LinkInfo> linkInfo(numLinks);
    uint idx = 0;
    size_t numSlashes;

    for (auto link : urls) {
        numSlashes = 0;
        auto start = link.rbegin();
        
        while (*start != '.'){
            if (*start=='/')
                ++numSlashes;
            ++start;
        }
        --start;
        switch (*start){
            case 'g': 
                linkInfo[idx++] = {link, 5, link.size(), numSlashes};
                break;
            case 'e':
                linkInfo[idx++] = {link, 4, link.size(), numSlashes};
                break;
            case 'c':
                linkInfo[idx++] = {link, 3, link.size(), numSlashes};
                break;
            case 'n':
                linkInfo[idx++] = {link, 2, link.size(), numSlashes};
                break;
            default:
                linkInfo[idx++] = {link, 1, link.size(), numSlashes};
                break;
        }
    }

    std::sort(linkInfo.begin(), linkInfo.end(), staticRankComparator);
    return linkInfo;
}



/*
    Ranker:
        What is the input?
            Links
            Query
        Where is the input?
        How are we recieving it?
        When are we recieving it?
        What do we want to return?
            Rank
                Rank factors + Final Rank
            Url
            Title - ask Michael/han how to get this?
            ~snippet~ - optional
    What we dont want to worry about:
*/