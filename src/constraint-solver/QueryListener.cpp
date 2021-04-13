/*
 * Query Listener
 * 
 * Runs on Each worker, responding to incoming queries with this worker's ranked results
 * 
 * Created by mjwolf on 4/12/2021
 */ 

#include "../stl/String.h"
#include "../query_parser/query_parser.h"
#include "QueryListener.h"
#include "ConstraintSolver.h"

// TODO remove
#include <string>

using std::cout;
using std::cerr;
using std::endl;

int main(int argc, char **argv) {

    // Usage
    if ( argc <= 2) {
        cerr << "Usage: " << argv[0] << " [max number of matches] [query]" << endl;
        return 1;
    }

    // Read match limit from args
    unsigned matchLimit = static_cast<unsigned>(atoi(argv[1]));

    // Declare match and rankings arrays
    DocumentMatch matches[matchLimit];
    unsigned rankingIndices[matchLimit];

    // Convert args to query
    scb::String query;
    for( int i = 2; i < argc; ++i) {
        query += argv[i];

        if(i < argc - 1) {
            query += " ";
        }
    }
    cout << "query: '" << query.cstr() << "'" << endl;
    cout << "match limit: " << matchLimit << std::endl;

    // Parse query
    Parser queryParser;
    // TODO remove once queryParser uses scb instead of std
    std::string query_str(query.cstr());
    AST *ast = queryParser(query_str);
    cout << "parsed: ";
    ast->print();
    cout<<"\n";


    // Run constraint solver
    ConstraintSolver::GetMatches(ast, matches, matchLimit);

    // TODO Rank
    // I think we would be best off ranking by setting the rankScore of each DocumentMatch
    // And then ordering them using a separate array of ordered indices (to avoid having to
    // Move whole objects around in an array when sorting)

    // Clean up
    delete ast;
    
    return 0;
}