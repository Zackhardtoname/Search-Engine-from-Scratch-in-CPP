#include "Ranker.h"
int main(){
    std::vector<std::string> statictestbench = {
        "https://google.com",
        "http://google.com",
        "https://whatev.gov/asdfas",
        "https://bing.gov",
        "https://killa_commodo.gov",
        "https://jesus.edu",
        "https://nevada.net",
        "https://lmao.com",
        "https://hallelujah.com",
        "https://remoty.gov",
        "https://jehotr.edu",
        "https://dihdi.ju",
        "https://sdfbaf.afn",
        "https://aifbusfd.ofwanupw",
        "https://aidipfav.f",
        "https://iobafoabf.iab",
        "https://akjdsfnl.net",
        "https://abfdlkfblab.com",
        "https://skndisndinsdisndind.gov"
    };
    Vector<String> dynamictestbench = {
        "https://googoogledisn.com",
        "http://google.com",
        "https://whatev.gov/asdfas",
        "https://bing.gov",
        "https://killa_commodo.gov",
        "https://jesus.edu",
        "https://nevada.net",
        "https://lmao.com",
        "https://hallelujah.com",
        "https://remoty.gov",
        "https://jehotr.edu",
        "https://dihdi.ju",
        "https://sdfbaf.afn",
        "https://aifbusfd.ofwanupw",
        "https://aidipfav.f",
        "https://iobafoabf.iab",
        "https://akjdsfnl.net",
        "https://abfdlkfblab.com",
        "https://skndisndinsdisndind.gov"
    };
    //www.geeksforgeeks.com
    // geeksforgeeks    com
    //  
    //Parser parser;
    /*
    auto ast = parser( "(ayy&lmao) -(given&plush-(hollandaise|youtube&isret))" );
    ast->print();
    dynamicRank(dynamictestbench, ast);


    auto k = staticRank(statictestbench);

    for (auto i : k)
        std::cout<<i.link<<"\n";
    std::cout<<"\n";

        "https://dihdi.ju",
        "https://sdfbaf.afn",
        "https://aifbusfd.ofwanupw"
        "https://aidipfav.f",
        "https://iobafoabf.iab",
        "https://akjdsfnl.net",
        "https://abfdlkfblab.com",
        "https://skndisndinsdisndind.gov"
    };
    */
    Parser parser;
    auto ast = parser( "goo jesus jesgo rem aidi akj disn" );
    //ast->print();
    
    //dynamicRank(dynamictestbench, ast);

    /*
    auto k = staticRank(statictestbench);

    for (auto i : k)
        std::cout<<i.link<<"\n";
    std::cout<<"\n";
    */
}

 