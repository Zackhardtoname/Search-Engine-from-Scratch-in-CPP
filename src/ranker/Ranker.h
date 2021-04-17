#include <bits/stdc++.h>
#include "Structures.h"
#include "../query-parser/QueryParser.h"
#include "../indexer/PorterStemming.h"
#include "aho-corasick.h"
#define toInt(c) (int)(((int)c) - (int)97)
// "harry potter and the goblet of fire"--initial test
// harry * potter * 
// best link types:
// .gov -> .edu -> .com -> .net -> .others
std::unordered_map<std::string, float> staticRankedLinks;
struct LinkInfo {
    std::string link;
    // Static ranking variables
    
    bool https;
    int domainType;
    size_t length;
    size_t numSlashes;
    
    // Final score as a result of static and dynamic ranking
    double score;
};

auto staticRankComparator(const LinkInfo &linkA, const LinkInfo &linkB) -> bool {
    if (linkA.https == linkB.https) {
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
    return linkA.https > linkB.https;
}

// TODO: Run the static rank once at the very beginning and memory map the rank values to a file
// We would compute the dynamic rank at runtime
std::vector<LinkInfo> staticRank(std::vector<std::string> urls) {
    size_t numLinks = urls.size();
    float linkFactor = 1/numLinks;
    std::vector<LinkInfo> linkInfo(numLinks);
    uint idx = 0;
    size_t numSlashes;
    int domainType;
    bool https;

    for (auto link : urls) {
        numSlashes = 0;
        https = false;
        auto start = link.rbegin();
        
        
        while (*start != '.'){
            if (*start=='/')
                ++numSlashes;
            ++start;
        }
        --start;

        if (link.at(4) == 's')
            https = true;

        switch (*start) {
            case 'g':
                domainType = 5;
                break;
            case 'e':
                domainType = 4;
                break;
            case 'c':
                domainType = 3;
                break;
            case 'n':
                domainType = 2;
                break;
            default:
                domainType = 1;
                break;
        }
        linkInfo[idx++] = {link, https, domainType, link.size(), numSlashes};
    }

    std::sort(linkInfo.begin(), linkInfo.end(), staticRankComparator);

    for (idx =0; idx<linkInfo.size(); ++idx)
        staticRankedLinks[linkInfo[idx].link] = linkInfo[idx].domainType/(linkFactor*idx); 
    return linkInfo;
}
struct ExternalBlackBox{
    std::string link;
};

// Dynamic Rank Factors
/*
    Consider ANDs and ORs as the same thing
    URLs - is the query in the URL?
        - URL is the only ranking factor where we worry about NOTs
        - Set rank to half of whatever it should if a not word is found
    Content - is the query in sequence in the document?
        - Consider ANDs
        - (cats*dogs) + (pigs*sheep) + birds (turn AST into prefix tree)
    Content - how many times is the query in sequence in the document?
    Content - is the query in the title?
    Content - how far apart are the query words in the document? (if not in sequence?)
    Content - individual word counts (the more repetitions the better)
    Content - does it have more of the rare words that were in the query?
    
*/

void nodeReturner(AST * Query, Vector<String> &queries, Vector<String> &disallow, bool sign){
    if(auto* literal = dynamic_cast<LiteralNode*>(Query)) {
        if(sign)
            queries.pushBack(literal->word);
        else
            disallow.pushBack(literal->word);
    } else if(auto* binary = dynamic_cast<BinaryOpNode*>(Query)) {
        nodeReturner(binary->left, queries, disallow, sign);
        if (binary->operation == '-')
            nodeReturner(binary->right, queries, disallow, !sign);
        else
            nodeReturner(binary->right, queries, disallow, sign);
    } else if(auto* unary = dynamic_cast<UnaryOpNode*>(Query)){
        if (unary->operation == '-')
            nodeReturner(unary->param, queries, disallow, !sign);
        else
            nodeReturner(unary->param, queries, disallow, sign);
    }
}
/*
urls need to be stemmed
    leg->legs
    just pass into pre-written function

now we have a littany of words
    //the longer the matched query string, the more it is weighted
Now we make the trie

*/
void dynamicRank(Vector<String> urls, AST * Query) {
    Trie trie;
    Vector<String> queries; 
    Vector<String> disallow;

    //convert the AST into a vector of words in 'queries', and all NOTs into 'disallow'
    nodeReturner(Query, queries, disallow, 1);

    //do porter stemming on all of the query words.
    //defined in indexer (../indexer/PorterStemming.h)
    for (int i = 0; i<q_sz; ++i)
        queries.pushBack(PorterStem(queries[i]));

    //Implement Aho-Corsick to reduce time to search URLs to linear with O(m + q)*O(n) with preprocessing rather than O(nlogn)*O(*q*d*n)
    std::set<int> found;
    Vector<bool> found(queries.size());
    Vector<double> dynRank(urls.size());

    for (int i = 0; i< queries.size(); ++i)
        trie.addPattern(queries[i].cstr(),i);
    uint urlNUM=0;
    for (auto url : urls){
        trie.processText(url.cstr(), found);
        for (auto it = found.begin(); it != found.end(); it++) {
            found[*it]=1;
        }
        for (int i = 0; i<found.size(); ++i)
            dynRank[urlNUM]+=i*found[i];
        ++urlNUM;
    }
    //found dynamic url rank.

    


    


    /*
    for(String url : urls) {
        
        std::string findWords(url.cstr());
        
        // Start at begin + 7 to avoid the https:// or http://
        //auto first = find_if(findWords.begin() + 7, findWords.end(), ::isalnum);
        //auto last = find_if_not(first, findWords.end(), ::isalnum);
        //while (first != last) {
        //    std::string newWord(first, last);
        //    urlWords.pushBack(newWord);
        //    first = find_if(last, findWords.end(), ::isalnum);
        //    last = find_if_not(first, findWords.end(), ::isalnum);
        //}

        //for(auto word : urlWords) {
        //    std::cout << word << '\t';
        //}
        //std::cout<<"\n";
    
        urlWords.clear();
    }
    */
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
