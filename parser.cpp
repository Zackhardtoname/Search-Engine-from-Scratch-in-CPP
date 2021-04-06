/*
 * parser.cpp
 *
 * Implementation of parser.h
 *
 * See parser.h for a full BNF of the grammar to implement
 *
 * Lab3: You should implement the different Find( ) functions,
 * as well as any additional functions you declare in parser.h
 */

#include "expression.h"
#include "parser.h"
#include <queue>


Expression *Parser::FindFactor( )
   {
   // TODO
   return nullptr;
   }

Expression *Parser::FindAdd( )
   {
   // TODO
   return nullptr;
   }

Expression *Parser::FindMultiply( )
   {
   // TODO5+(9+3+5)-67-(7654+654)
   return nullptr;
   }

int Parser::Parse( )
   {
   struct numCharacteristics{
      bool lA;
      int prec;
   };
   
   std::unordered_map<std::string, numCharacteristics> NumT;

   NumT["+"] = {false, 0};
   NumT["-"] = {true, 0};
   NumT["*"] = {false, 1};
   NumT["/"] = {true, 1};

   enum Types { num=0, op=1, lpar=2, rpar=3, func=4};
   //given string in "in", create PEMDAS style N-d array
   const char * str = _in.c_str();
   if (_in.size()==0){
      std::cout << "Syntax error\n";
      exit(0);
   }
   if (_in.size()==1){
      if (isdigit(_in[0]))
         return std::stoi(_in);
      else{
         std::cout << "Syntax error\n";
         exit(0);
      }
            
   }
   std::string currChar;
   std::queue<std::string> output_queue;
   std::stack<std::string> operator_stack;
   //while there are tokens to be read

   if (!isdigit(*str) && *str!='-' && *str!='('){
      std::cout << "Syntax error\n";
      exit(0);
   }

   int lastOpLoc = 0;
   int lastNumLoc = -1;

   for (int strIdx = 0; strIdx<_in.size(); ++strIdx)
      {
      //read a token
      currChar.clear();
      currChar += *str;
      ++str;

      if (currChar==" ")
         continue;
      while (isdigit(currChar[0])&&isdigit(*str))
         currChar+=*str, ++str, ++strIdx; 
      
      auto charType = [&](){
         //if its a number
         if (isdigit(currChar[0]))
            return num;
         else if (currChar=="+" || currChar=="-" || currChar=="*" || currChar=="/")
            return op;
         else if (currChar=="(")
            return lpar;
         else if (currChar==")")
            return rpar;
         else{
            std::cout << "Syntax error\n";
            exit(0);
         }
      };

      auto precedence_calc = [&](){
         auto op =operator_stack.top();
         auto tok =currChar; 
         //operator on top of stack not lpar
         if (op!="("){
            //operator on stack has greater precedence
            if (NumT[op].prec>NumT[tok].prec)
               return true;
            //operators have equal precedence, and token is left associative
            else if((NumT[op].prec==NumT[tok].prec) && NumT[tok].lA)
               return true;
         }
         return false;
      };

      int currType = charType();

      //std::cout<<currType<<"\n";

      switch(currType)
         {
         case num:
            lastNumLoc++;
            output_queue.push(currChar);
            break;
         case op:
            lastOpLoc++;
            if (lastOpLoc-lastNumLoc>=2){
               std::cout << "Syntax error\n";
               exit(0);
            }

            if (!operator_stack.empty()){
               while (!operator_stack.empty() && precedence_calc())
                  output_queue.push(operator_stack.top()), operator_stack.pop();
            }
            operator_stack.push(currChar);
            break;
         case lpar:
            operator_stack.push(currChar);
            break;
         case rpar:
            while(operator_stack.top()!="("){
               output_queue.push(operator_stack.top()), operator_stack.pop();
               if (operator_stack.empty())
                  break;
            }
            if (operator_stack.empty())
               {
               std::cout << "Syntax error\n";
               exit(0);
               }
            if (operator_stack.top()=="(")
               operator_stack.pop();
            else
            {
            std::cout << "Syntax error\n";
            exit(0);
            }
            break;
            
         default:
            exit(0);
         }

      }
      while (!operator_stack.empty())
         {
            if(operator_stack.top()=="(" || operator_stack.top()==")"){
               std::cout << "Syntax error\n";
               exit(0);
            }
            output_queue.push(operator_stack.top()), operator_stack.pop();
         }


      //while (!output_queue.empty()){
      //   std::cout<<output_queue.front()<<" ";
      //   output_queue.pop();
      //}
      //exit(0);
      int rhs, lhs;
      while (!output_queue.empty()){
         //std::cout<<output_queue.front()<<" ";
         if (isdigit(output_queue.front()[0]))
            operator_stack.push(output_queue.front()), output_queue.pop();
         else if (output_queue.front()=="+"){
            if (operator_stack.empty()){
               std::cout << "Syntax error\n";
               exit(0);
            }
            lhs = std::stoi(operator_stack.top()), operator_stack.pop();
            if (operator_stack.empty()){
               std::cout << "Syntax error\n";
               exit(0);
            }

            rhs = std::stoi(operator_stack.top()), operator_stack.pop();
            
            operator_stack.push(std::to_string(rhs+lhs)), output_queue.pop();
         }
         else if (output_queue.front()=="-"){
            if (operator_stack.empty()){
               std::cout << "Syntax error\n";
               exit(0);
            }
            lhs = std::stoi(operator_stack.top()), operator_stack.pop();
            if (operator_stack.empty())
               rhs = 0;
            else
               rhs = std::stoi(operator_stack.top()), operator_stack.pop();
            operator_stack.push(std::to_string(rhs-lhs)), output_queue.pop();
         }
         else if (output_queue.front()=="*"){
            if (operator_stack.empty()){
               std::cout << "Syntax error\n";
               exit(0);
            }
            lhs = std::stoi(operator_stack.top()), operator_stack.pop();
            if (operator_stack.empty()){
               std::cout << "Syntax error\n";
               exit(0);
            }
            rhs = std::stoi(operator_stack.top()), operator_stack.pop();
            operator_stack.push(std::to_string(lhs*rhs)), output_queue.pop();
         }
         else if (output_queue.front()=="/"){
            if (operator_stack.empty()){
               std::cout << "Syntax error\n";
               exit(0);
            }
            lhs = std::stoi(operator_stack.top()), operator_stack.pop();
            if (operator_stack.empty()){
               std::cout << "Syntax error\n";
               exit(0);
            }
            rhs = std::stoi(operator_stack.top()), operator_stack.pop();
            
            operator_stack.push(std::to_string(rhs/lhs)), output_queue.pop();
         }
      }
      if (operator_stack.size()>1){
         std::cout << "Syntax error\n";
         exit(0);
      }

      return std::stoi(operator_stack.top());
      
   }

Parser::Parser( const std::string &in ) :
      _in( in )
   {
      ;
   }
