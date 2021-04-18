#include <bits/stdc++.h>
#include "../query-parser/QueryParser.h"
#include "../indexer/PorterStemming.h"
#include "../constraint-solver/ISR/ISR.h"
#include "../constraint-solver/QueryableChunk.h"
#include "aho-corasick.h"
#include <pthread.h>
#include <math.h>
#define toInt(c) (int)(((int)c) - (int)97)
// "harry potter and the goblet of fire"--initial test
// harry * potter * 
// best link types:
// .gov -> .edu -> .com -> .net -> .others
class StringHasher{
    public:
        size_t operator()(const String & s) const{
            return s.hash();
        }
};
std::unordered_map<String, float, StringHasher> staticRankedLinks;

struct LinkInfo {
    String link;
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
Vector<LinkInfo> staticRank(Vector<String> urls) {
    size_t numLinks = urls.size();
    float linkFactor = 1/numLinks;
    Vector<LinkInfo> linkInfo(numLinks);
    uint idx = 0;
    size_t numSlashes;
    int domainType;
    bool https;

    for (auto link : urls) {
        numSlashes = 0;
        https = false;
        auto start = link.end();
        
        
        while (*start != '.'){
            if (*start=='/')
                ++numSlashes;
            --start;
        }
        ++start;

        if (link[4] == 's')
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

void nodeReturner(AST * Query, Vector<String> &queries/*, Vector<String> &disallow*/, bool sign){
    if(auto* literal = dynamic_cast<LiteralNode*>(Query)) {
        if(sign)
            queries.pushBack(literal->word);
        //else
        //    disallow.pushBack(literal->word);
    } else if(auto* binary = dynamic_cast<BinaryOpNode*>(Query)) {
        nodeReturner(binary->left, queries/*, disallow*/, sign);
        if (binary->operation == '-')
            nodeReturner(binary->right, queries, /*disallow,*/ !sign);
        else
            nodeReturner(binary->right, queries, /*disallow,*/ sign);
    } else if(auto* unary = dynamic_cast<UnaryOpNode*>(Query)){
        if (unary->operation == '-')
            nodeReturner(unary->param, queries/*, disallow*/, !sign);
        else
            nodeReturner(unary->param, queries, /*disallow,*/ sign);
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
//a b c
//an be co
// an b co
// a||an b||be c||co

// how to toot your own horn
// to toot your own horn

std::map<uint32_t,int> shared_isr_map;
std::unordered_map<String, int, StringHasher> strMap;
std::unordered_map<int,int> intOrder;

struct sharedISRStruct {
    ISR * isr;
    int query;
    uint32_t edge;
};

//concatenate a cat

void * rankerSearcher (void * arg){
    sharedISRStruct * currStruct = (sharedISRStruct *) arg;
    auto * isr = currStruct->isr;
    auto query =  currStruct->query;
    auto edge = currStruct->edge;

    while (isr->Get()<edge){
        shared_isr_map[isr->Get()] = query;
        isr->Next();
    }

    //isr.Get();
    //gets a specific ISR, and the specific word it is pertaining to.
    //looks for instances of the word

    //if instance is found
        //get location of instance from ISR, call it idx
        //lock the shared structure
        //if the current word is longer than the one stored, replace it
            //lengths provided in intOrder from strMap.
        //else dont
        //unlock
    //keep searching.
}


void dynamicRank(Vector<String> urls, AST * Query, Vector<QueryableChunk*> chunks, Vector<size_t> docID) {
    Trie trie;
    Vector<String> queries; 
    
    //convert the AST into a vector of words in 'queries', and all NOTs into 'disallow'
    nodeReturner(Query, queries, 1);

    //do porter stemming on all of the query words.
    //defined in indexer (../indexer/PorterStemming.h)
    uint qsz = queries.size();
    Vector<String> stemmed(queries.size());
    for (int i = 0; i<qsz; ++i)
        stemmed[i] = PorterStem(queries[i]), strMap[stemmed[i]]=i, 
            intOrder[i]=stemmed[i].size();
    
    // Content Ranking
    // if query is in the document
        // whole query
            // number of consecutive matches
                // consecutive meaning allowed to alternate between steemed and vanilla
                    // e.g. ("cat and dog" would find perfect match with "cats and dogs")
                // Scoring:
                    // Number of words in a row consecutively add more
                        // Given query: "I want to die"
                            // If the webpage contains "I want to die"
                                // The score is:
                                    // \Sigma_{k=1}^{query.size()}{Occ[k] * e^{-(query.size()-k)}} 
                    // Also counting number of times words show up.
                        // create a mapping of words, and increment counter when they show up
                            // after sorting all queries, at the end, multiply by length

    //CONTENT RANKING
    //needs to be highly multithreaded.
    //first implement naively, then go back and multithread.


    // ISR for every word
        // all ISRs search till end at same time
        // insert into shared structure location, idx of what they found queries[0]->0 etc.
        // allows for super quick search, number of occurences, and consecutive terms all at the same time.
        // In o(n)

    //this for loop is actually a shit ton of threads, and this isnt saved in this function
    //all done simultaneously 
    std::vector<double> contentRank(urls.size());
    
    Vector<float> savedExponent;
    for (int i = 1; i<=stemmed.size(); ++i){
        savedExponent.pushBack(exp(-(queries.size()-i)));
    }

    for (int i = 0; i<urls.size(); ++i){
        auto currUrl = urls[i];
        auto currChunk = chunks[i];
        auto currID = docID[i];
        //create an isr for each word, corresponding to the current document
        for (auto query : stemmed){
            AST * ast = new LiteralNode(query);
            auto isr = currChunk->MakeISR(ast);
            // we want the isr to be on the location of the first matching query
                //Starting at the beginning of the document specified.
            
            auto edge = isr->chunk.LastPostTable[currID]; 

            if (!currID)
                isr->Seek(isr->chunk.LastPostTable[0]);  
            else  
                isr->Seek(isr->chunk.LastPostTable[currID-1]);

            //hasnt fallen off the edge
            if (isr->Get()<=edge){
                struct sharedISRStruct * args =new sharedISRStruct {isr, strMap[query], edge};
                //pthread_create(&cThread, NULL, rankerSearcher, (void*)&args);
                rankerSearcher((void*)&args);
            }
            
            //join threads

            uint32_t prevKey = shared_isr_map.begin()->first;
            int      prevVal = shared_isr_map.begin()->second;
            uint     currLen = 1;

            for (auto occurence : shared_isr_map){
                // num occurences
                // all in order.
                //  all-1 in order, all-2 in order, etc. etc.
                
                if (occurence.first-prevKey == 1){
                    if (occurence.second-prevVal == 1){
                        ++currLen;
                        contentRank[i]+=savedExponent[currLen-1]*currLen;
                    }
                }
                else{
                    currLen=1;
                }
                //length of the string
                //TODO figure out weighting
                contentRank[i]+=intOrder[occurence.second]/100.0;
            }
            //pthread_t cThread;
            //actually give it shit
        }
        //join threads.
    }
    



    //URL RANKING
    for (auto stem : stemmed)
        queries.pushBack(stem);
    std::sort(queries.begin(), queries.end());

    //Implement Aho-Corsick to reduce time to search URLs to linear with O(m + q)*O(n) with preprocessing rather than O(nlogn)*O(*q*d*n)
    std::set<int> found;
    Vector<bool> _found(queries.size());
    Vector<double> urlRank(urls.size());

    for (int i = 0; i< queries.size(); ++i)
        trie.addPattern(queries[i].cstr(),i);

    uint urlNUM=0;

    for (auto url : urls){
        trie.processText(url.cstr(), found);
        for (auto it = found.begin(); it != found.end(); it++) {
            _found[*it]=1;
        }
        for (int i = 0; i<_found.size(); ++i)
            urlRank[urlNUM]+=i*_found[i], _found[i]=0;
        ++urlNUM;
    }

    for (int i = 0; i< urls.size(); ++i){
        std::cout<< urls[i] <<":\t"<<urlRank[i]<<"\n";
    }



    //found dynamic url rank.
    // i want to learn computer science
    


    


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
