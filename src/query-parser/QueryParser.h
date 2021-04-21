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

inline double str_to_num( const String &str )
   {
   double ll;
   try
      {
      // Hamilton has said we should prioritize 'asking for forgiveness' over
      // 'asking for permission', so I have left the use of std::stod (string to
      // double) here, unless someone else wants to write a function that
      // handles this.
      // TODO: Write our own version of std::stod
      std::string temp = str.cstr();
      ll = std::stod( temp );
      return ll;
      }
   catch ( std::exception e )
      {
      std::cout << "Syntax Error: Failure to convert string to double\n";
      exit( 1 );
      }
   }

inline String num_to_str( const double num )
   {
   std::ostringstream o;
   o << num;
   return o.str().c_str();
   }

enum class Token
{
   literal,
   OR = '|',
   sub = '-',
   AND = '&',
   lpar = '(',
   rpar = ')',
   div = '/',
   eof = -1,
   str
};

class Lexer
   {
   public:
      Token current_token;
      String curr_text;
      std::istream *lexer_input_stream;

      explicit Lexer( std::istream &_is ) : lexer_input_stream{ &_is }
         {
         current_token = getTok(), curr_text = buff;
         is_beginning = false;
         }

      explicit Lexer( std::istream *_is ) : lexer_input_stream{ _is }
         {
         current_token = getTok(), curr_text = buff;
         is_beginning = false;
         }

      bool last_token_was_operator = false;
      bool is_beginning = true;
      Token getTok()
         {
         auto &in = *lexer_input_stream;
         buff.clear();
         auto c = in.get();

         if ( isspace( c ) )
            {
            while ( isspace( c ) )
               c = in.get();
            if ( is_beginning )
               {
               ;
               }
            else if ( !in )
               {
               return Token::eof;
               }
            else if ( c == '|' || c == '-' || c == '&' || c == '/' ||
                      c == '(' || c == ')' )
               {
               buff = c;
               last_token_was_operator = true;
               return Token( c );
               }
            else if ( last_token_was_operator )
               {
               ;
               }
            else
               {
               in.putback( c );
               c = '&';
               buff = '&';
               return Token::AND;
               }
            }
         // EOF
         if ( !in )
            return Token::eof;
         buff = c;
         if ( c == '|' || c == '-' || c == '&' || c == '/' || c == '(' ||
              c == ')' )
            {
            last_token_was_operator = true;
            return Token( c );
            }
         c = in.get();
         while ( !( c == '|' || c == '-' || c == '&' || c == '/' || c == '(' ||
                    c == ')' || c == ' ' ) &&
                 in )
            {
            buff += c, c = in.get();
            }
         in.putback( c );
         last_token_was_operator = false;
         return Token::str;
         }

      void nextTok()
         {
         if ( current_token != Token::eof )
            current_token = getTok(), curr_text = buff;
         }

      String buff;
   };

class AST
   {
   public:
      virtual void print() = 0;
   };

class BinaryOpNode : public AST
   {
   public:
      AST *left, *right;
      char operation;

      void print()
         {
         std::cout << "(";
         left->print();
         std::cout << operation;
         right->print();
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

      void print()
         {
         std::cout << "(";
         std::cout << operation;
         param->print();
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
      void print()
         {
         std::cout << word.cstr();
         }

      LiteralNode( String n ) : word( std::move( n ) )
         {
         }
   };

// An LR(1) parser
// Reduction rules:
// 0. num -> wrapped num
// 1. (num) -> num
// 2. a*/b -> Mult/Div(a, b)
// 3. a+-b (lookahead != */) -> Add/Sub(a, b)
// 4. +-a -> Pos/Neg(a) (lookahead != +-*/)
class Parser
   {
      struct Symbol
         {
            bool isTerminal; // if is terminal
            union
               {
                  Token terminal;
                  AST *nonTerminal;
               } data;

            bool is( Token tok )
               {
               return isTerminal && data.terminal == tok;
               }

            Symbol( AST *ast )
               {
               isTerminal = false;
               data.nonTerminal = ast;
               }

            Symbol( Token tok )
               {
               isTerminal = true;
               data.terminal = tok;
               }
         };

   public:
      AST *operator()( const String &s )
         {
         std::istringstream parser_input_stream{ s.cstr() };
         Lexer lex{ parser_input_stream };

         Vector<Symbol> stack;

         Vector<Pair<Token, String>> tokens;
         while ( lex.current_token != Token::eof )
            {
            tokens.emplaceBack( lex.current_token, lex.curr_text );
            lex.nextTok();
            }

         while ( true )
            {
            int sz = stack.size();
            if ( sz >= 3 && stack[ sz - 3 ].is( Token::lpar ) &&
                 !stack[ sz - 2 ].isTerminal &&
                 stack[ sz - 1 ].is( Token::rpar ) )
               {
               // rule 1. (num)
               auto sym = stack[ sz - 2 ];
               stack.popBack();
               stack.popBack();
               stack.popBack();
               stack.pushBack( sym );
               }
            else if ( sz >= 3 && !stack[ sz - 3 ].isTerminal &&
                      ( stack[ sz - 2 ].is( Token::AND ) ||
                        stack[ sz - 2 ].is( Token::div ) ) &&
                      !stack[ sz - 1 ].isTerminal )
               {
               // rule 2. a*/b
               auto left = stack[ sz - 3 ];
               char operation = (char)stack[ sz - 2 ].data.terminal;
               auto right = stack[ sz - 1 ];
               auto *node = new BinaryOpNode(
                  left.data.nonTerminal, right.data.nonTerminal, operation );
               stack.popBack();
               stack.popBack();
               stack.popBack();
               stack.emplaceBack( node );
               }
            else if ( sz >= 3 && !stack[ sz - 3 ].isTerminal &&
                      ( stack[ sz - 2 ].is( Token::OR ) ||
                        stack[ sz - 2 ].is( Token::sub ) ) &&
                      !stack[ sz - 1 ].isTerminal &&
                      ( tokens.empty() ||
                        ( tokens[ 0 ].first != Token::AND &&
                          tokens[ 0 ].first != Token::div ) ) )
               {
               // rule 3. a+-b (lookahead != */)
               auto left = stack[ sz - 3 ];
               char operation = (char)stack[ sz - 2 ].data.terminal;
               auto right = stack[ sz - 1 ];
               auto *node = new BinaryOpNode(
                  left.data.nonTerminal, right.data.nonTerminal, operation );
               stack.popBack();
               stack.popBack();
               stack.popBack();
               stack.emplaceBack( node );
               }
            else if ( ( sz == 2 ||
                        ( sz >= 3 && stack[ sz - 3 ].isTerminal ) ) &&
                      ( stack[ sz - 2 ].is( Token::OR ) ||
                        stack[ sz - 2 ].is( Token::sub ) ) &&
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
               if ( tokens[ 0 ].first == Token::literal ||
                    tokens[ 0 ].first == Token::str )
                  stack.emplaceBack( new LiteralNode( tokens[ 0 ].second ) );
               else
                  stack.emplaceBack( tokens[ 0 ].first );

               tokens.erase( 0 );
               }
            }

         if ( stack.size() != 1 || stack[ 0 ].isTerminal )
            {
            std::cout
               << "Syntax Error: Stack is corrupted. Could be due to "
                  "incomplete parentheses or another formatting issue.\n";
            exit( 1 );
            }

         return stack[ 0 ].data.nonTerminal;
         }
   };

#endif // QUERY_PARSER_H
