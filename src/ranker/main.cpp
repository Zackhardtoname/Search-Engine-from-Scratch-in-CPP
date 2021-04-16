#include "Ranker.h"


int main(){
    std::vector<std::string> testbench = {
        "google.com",
        "whatev.gov/asdfas",
        "bing.gov",
        "killa_commodo.gov",
        "jesus.edu",
        "nevada.net",
        "lmao.com",
        "hallelujah.com",
        "remoty.gov",
        "jehotr.edu",
        "dihdi.ju",
        "sdfbaf.afn",
        "aifbusfd.ofwanupw"
        "aidipfav.f",
        "iobafoabf.iab",
        "akjdsfnl.net",
        "abfdlkfblab.com",
        "skndisndinsdisndind.gov"
    };
    auto k = static_rank(testbench);

    for (auto i : k)
        std::cout<<i.link<<"\n";
    std::cout<<"\n";

    std::cout<<"it works\n";
}