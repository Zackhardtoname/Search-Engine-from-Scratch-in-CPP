#ifndef QUERY_PARSER_H
#define QUERY_PARSER_H

#include "../stl/Pair.h"
#include "../stl/String.h"
#include "../stl/Vector.h"
#include <algorithm>
#include <bits/stdc++.h>
#include <cctype>
#include <cmath>
#include <exception>
#include <iostream>
#include <sstream>

using scb::Pair;
using scb::String;
using scb::Vector;

enum class QueryToken
{
   Or = '|',
   Sub = '-',
   And = '&',
   LPar = '(',
   RPar = ')',
   Div = '/',
   Eof = -1,
   Str
};

class Lexer
   {
      String buff;

   public:
      QueryToken curToken;
      String curText;

      std::stringstream inputStream;

      explicit Lexer( const String &s ) : inputStream( s.cstr() )
         {
         curToken = getTok(), curText = buff;
         isBeginning = false;
         }

      bool lastTokenWasOperator = false;
      bool isBeginning = true;

      QueryToken getTok()
         {
         buff.clear();
         char c = inputStream.get();

         if ( isspace( c ) )
            {
            while ( isspace( c ) )
               c = inputStream.get();

            if ( isBeginning || lastTokenWasOperator )
               ;
            else if ( !inputStream )
               return QueryToken::Eof;
            else if ( c == '|' || c == '-' || c == '&' || c == '/' ||
                      c == '(' || c == ')' )
               {
               buff = static_cast<char>( c );
               lastTokenWasOperator = true;
               return QueryToken( c );
               }
            else
               {
               inputStream.putback( c );
               buff = '&';
               return QueryToken::And;
               }
            }
         // EOF
         if ( !inputStream )
            return QueryToken::Eof;
         buff = c;
         if ( c == '|' || c == '-' || c == '&' || c == '/' || c == '(' ||
              c == ')' )
            {
            lastTokenWasOperator = true;
            return QueryToken( c );
            }
         c = inputStream.get();
         while ( !( c == '|' || c == '-' || c == '&' || c == '/' || c == '(' ||
                    c == ')' || c == ' ' ) &&
                 inputStream )
            {
            buff += c, c = inputStream.get();
            }
         inputStream.putback( c );
         lastTokenWasOperator = false;
         return QueryToken::Str;
         }

      void nextTok()
         {
         if ( curToken != QueryToken::Eof )
            curToken = getTok(), curText = buff;
         }
   };

class AST
   {
   public:
      virtual void Print() = 0;
   };

class BinaryOpNode : public AST
   {
   public:
      AST *left, *right;
      char operation;

      void Print()
         {
         std::cout << "(";
         left->Print();
         std::cout << operation;
         right->Print();
         std::cout << ")";
         }

      BinaryOpNode( AST *left, AST *right, char op )
         {
         this->left = left;
         this->right = right;
         this->operation = op;
         }
   };

class UnaryOpNode : public AST
   {
   public:
      char operation;
      AST *param;

      void Print()
         {
         std::cout << "(";
         std::cout << operation;
         param->Print();
         std::cout << ")";
         }

      UnaryOpNode( char op, AST *p )
         {
         operation = op;
         param = p;
         }
   };

class LiteralNode : public AST
   {
   public:
      String word;

      void Print()
         {
         std::cout << word.cstr();
         }

      LiteralNode( String n ) : word( std::move( n ) )
         {
         }
   };

struct QuerySymbol
   {
      bool isTerminal; // if is terminal
      union
         {
            QueryToken terminal;
            AST *nonTerminal;
         } data;

      bool is( QueryToken tok )
         {
         return isTerminal && data.terminal == tok;
         }

      QuerySymbol( AST *ast )
         {
         isTerminal = false;
         data.nonTerminal = ast;
         }

      QuerySymbol( QueryToken tok )
         {
         isTerminal = true;
         data.terminal = tok;
         }
   };

// An LR(1) parser
// Reduction rules:
// 0. num -> wrapped num
// 1. (num) -> num
// 2. a*/b -> Mult/Div(a, b)
// 3. a+-b (lookahead != */) -> Add/Sub(a, b)
// 4. +-a -> Pos/Neg(a) (lookahead != +-*/)
AST *ParseQuery( const String &s )
   {
   Lexer lex( s );

   Vector<QuerySymbol> stack;

   Vector<Pair<QueryToken, String>> tokens;
   while ( lex.curToken != QueryToken::Eof )
      {
      tokens.emplaceBack( lex.curToken, lex.curText );
      lex.nextTok();
      }

   while ( true )
      {
      int sz = stack.size();
      if ( sz >= 3 && stack[ sz - 3 ].is( QueryToken::LPar ) &&
           !stack[ sz - 2 ].isTerminal &&
           stack[ sz - 1 ].is( QueryToken::RPar ) )
         {
         // rule 1. (num)
         auto sym = stack[ sz - 2 ];
         stack.popBack();
         stack.popBack();
         stack.popBack();
         stack.pushBack( sym );
         }
      else if ( sz >= 3 && !stack[ sz - 3 ].isTerminal &&
                ( stack[ sz - 2 ].is( QueryToken::And ) ||
                  stack[ sz - 2 ].is( QueryToken::Div ) ) &&
                !stack[ sz - 1 ].isTerminal )
         {
         // rule 2. a*/b
         auto left = stack[ sz - 3 ];
         char operation = (char)stack[ sz - 2 ].data.terminal;
         auto right = stack[ sz - 1 ];
         auto *node = new BinaryOpNode( left.data.nonTerminal,
                                        right.data.nonTerminal, operation );
         stack.popBack();
         stack.popBack();
         stack.popBack();
         stack.emplaceBack( node );
         }
      else if ( sz >= 3 && !stack[ sz - 3 ].isTerminal &&
                ( stack[ sz - 2 ].is( QueryToken::Or ) ||
                  stack[ sz - 2 ].is( QueryToken::Sub ) ) &&
                !stack[ sz - 1 ].isTerminal &&
                ( tokens.empty() || ( tokens[ 0 ].first != QueryToken::And &&
                                      tokens[ 0 ].first != QueryToken::Div ) ) )
         {
         // rule 3. a+-b (lookahead != */)
         auto left = stack[ sz - 3 ];
         char operation = (char)stack[ sz - 2 ].data.terminal;
         auto right = stack[ sz - 1 ];
         auto *node = new BinaryOpNode( left.data.nonTerminal,
                                        right.data.nonTerminal, operation );
         stack.popBack();
         stack.popBack();
         stack.popBack();
         stack.emplaceBack( node );
         }
      else if ( ( sz == 2 || ( sz >= 3 && stack[ sz - 3 ].isTerminal ) ) &&
                ( stack[ sz - 2 ].is( QueryToken::Or ) ||
                  stack[ sz - 2 ].is( QueryToken::Sub ) ) &&
                !stack[ sz - 1 ].isTerminal )
         {
         // rule 4. +- a
         char operation = (char)stack[ sz - 2 ].data.terminal;
         auto *param = stack[ sz - 1 ].data.nonTerminal;
         auto *node = new UnaryOpNode( operation, param );
         stack.popBack();
         stack.popBack();
         stack.emplaceBack( node );
         }
      else if ( tokens.empty() )
         {
         break;
         }
      else
         {
         // rule 0
         if ( tokens[ 0 ].first == QueryToken::Str )
            stack.emplaceBack( new LiteralNode( tokens[ 0 ].second ) );
         else
            stack.emplaceBack( tokens[ 0 ].first );

         tokens.erase( 0 );
         }
      }

   if ( stack.size() != 1 || stack[ 0 ].isTerminal )
      {
      std::cout << "Syntax Error: Stack is corrupted. Could be due to "
                   "incomplete parentheses or another formatting issue.\n";
      exit( 1 );
      }

   return stack[ 0 ].data.nonTerminal;
   }

#endif // QUERY_PARSER_H
