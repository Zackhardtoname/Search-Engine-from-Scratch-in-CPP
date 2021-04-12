#include "QueryParser.h"

int main() {
    Parser parser;
    std::string s;
    std::getline(std::cin, s);

    auto ast = parser(s);
    ast->print();
    std::cout<<"\n";
    return 0;
}
