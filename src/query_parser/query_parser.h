#include <bits/stdc++.h>
#include <iostream>
#include <sstream>
#include <map>
#include <string>
#include <algorithm>
#include <cctype>
#include <cmath>
#include <exception>

inline double str_to_num(const std::string &str) {
    double ll;
    try {
        ll = std::stod(str);
        return ll;
    }
    catch (std::exception e) {
        std::cout << "Syntax error B\n";
        exit(1);
    }
}

inline std::string num_to_str(const double num) {
    std::ostringstream o;
    o << num;
    return o.str();
}

enum class Token {
    literal, plus = '+', sub = '-', mul = '*', lpar = '(', rpar = ')', div = '/', eof = -1, str
};

class Lexer {
public:
    Token current_token;
    std::string curr_text;
    std::istream *is;

    explicit Lexer(std::istream &_is) : is{&_is} {
        current_token = getTok(), curr_text = buff;
        isBeginning=false;
    }

    explicit Lexer(std::istream *_is) : is{_is} {
        current_token = getTok(), curr_text = buff;
        isBeginning=false;

    }
    //~lexer(){delete is;}

    bool lastTokWasOp=false;
    bool isBeginning = true;
    Token getTok() {
        auto &in = *is;
        buff.clear();
        auto c = in.get();
        // + head
        if (isspace(c)){
            while (isspace(c))
                c = in.get();
            if (isBeginning){
                ;
            }
            else if (!in){
                return Token::eof;
            }
            else if (c == '+' || c == '-' || c == '*' || c == '/' || c == '(' || c == ')'){
                buff = c;
                lastTokWasOp=true;
                return Token(c);
            }
            else if (lastTokWasOp){
                ;
            }
            else{
                in.putback(c);
                c = '*';
                buff = '*';
                return Token::mul;
            }
        }
        //EOF
        if (!in)
            return Token::eof;
        buff = c;
        if (c == '+' || c == '-' || c == '*' || c == '/' || c == '(' || c == ')') {
            lastTokWasOp=true;
            return Token(c);
        }
        else{
            c = in.get();
            while (!(c == '+' || c == '-' || c == '*' || c == '/' || c == '(' || c == ')' || c == ' ') && in){
                buff+=c, c=in.get();
            }
            in.putback(c);
            lastTokWasOp=false;
            return Token::str;
        }
        std::cout << "Syntax error A\n";
        exit(1);
    }

    void nextTok() {
        if (current_token != Token::eof)
            current_token = getTok(), curr_text = buff;
    }

    std::string buff;
};

class AST {
public:
    virtual void print() = 0;
};

class BinaryOpNode : public AST {
public:
    AST *left, *right;
    char op;

    void print() {
        std::cout << "(";
        left->print();
        std::cout << op;
        right->print();
        std::cout << ")";
    }

    BinaryOpNode(AST *left, AST *right, char o) {
        this->left = left;
        this->right = right;
        this->op = o;
    }
};

class UnaryOpNode : public AST {
public:
    char op;
    AST *param;

    void print() {
        std::cout << "(";
        std::cout << op;
        param->print();
        std::cout << ")";
    }

    UnaryOpNode(char o, AST *p) {
        op = o;
        param = p;
    }
};

class LiteralNode : public AST {
public:
    //double num;
    //not a num, a word.
    std::string num;
    void print() {
        std::cout << num;
    }

    LiteralNode(std::string n) {
        num = n;
    }
};

// An LR(1) parser
// Reduction rules:
// 0. num -> wrapped num
// 1. (num) -> num
// 2. a*/b -> Mult/Div(a, b)
// 3. a+-b (lookahead != */) -> Add/Sub(a, b)
// 4. +-a -> Pos/Neg(a) (lookahead != +-*/)
class Parser {
    struct Symbol {
        bool isTerminal; // if is terminal
        union {
            Token terminal;
            AST *nonTerminal;
        } data;

        bool is(Token tok) {
            return isTerminal && data.terminal == tok;
        }

        Symbol(AST *ast) {
            isTerminal = false;
            data.nonTerminal = ast;
        }

        Symbol(Token tok) {
            isTerminal = true;
            data.terminal = tok;
        }
    };

public:
    AST *operator()(const std::string &s) {
        std::istringstream is{s};
        Lexer lex{is};

        std::vector<Symbol> stack;

        std::deque<std::pair<Token, std::string>> tokens;
        while (lex.current_token != Token::eof) {
            tokens.emplace_back(lex.current_token, lex.curr_text);
            lex.nextTok();
        }

        while (true) {
            int sz = stack.size();
            if (sz >= 3
                && stack[sz - 3].is(Token::lpar)
                && !stack[sz - 2].isTerminal
                && stack[sz - 1].is(Token::rpar)) {
                // rule 1. (num)
                auto sym = stack[sz - 2];
                stack.pop_back();
                stack.pop_back();
                stack.pop_back();
                stack.push_back(sym);
            } else if (sz >= 3
                       && !stack[sz - 3].isTerminal
                       && (stack[sz - 2].is(Token::mul) || stack[sz - 2].is(Token::div))
                       && !stack[sz - 1].isTerminal) {
                // rule 2. a*/b
                auto left = stack[sz - 3];
                char op = (char) stack[sz - 2].data.terminal;
                auto right = stack[sz - 1];
                auto *node = new BinaryOpNode(left.data.nonTerminal, right.data.nonTerminal, op);
                stack.pop_back();
                stack.pop_back();
                stack.pop_back();
                stack.emplace_back(node);
            } else if (sz >= 3
                       && !stack[sz - 3].isTerminal
                       && (stack[sz - 2].is(Token::plus) || stack[sz - 2].is(Token::sub))
                       && !stack[sz - 1].isTerminal
                       && (tokens.empty() || (tokens[0].first != Token::mul && tokens[0].first != Token::div))
                    ) {
                // rule 3. a+-b (lookahead != */)
                auto left = stack[sz - 3];
                char op = (char) stack[sz - 2].data.terminal;
                auto right = stack[sz - 1];
                auto *node = new BinaryOpNode(left.data.nonTerminal, right.data.nonTerminal, op);
                stack.pop_back();
                stack.pop_back();
                stack.pop_back();
                stack.emplace_back(node);
            } else if ((sz == 2 || (sz >= 3 && stack[sz - 3].isTerminal))
                       && (stack[sz - 2].is(Token::plus) || stack[sz - 2].is(Token::sub))
                       && !stack[sz - 1].isTerminal) {
                // rule 4. +- a
                char op = (char) stack[sz - 2].data.terminal;
                auto *param = stack[sz - 1].data.nonTerminal;
                auto *node = new UnaryOpNode(op, param);
                stack.pop_back();
                stack.pop_back();
                stack.emplace_back(node);
            } else if (tokens.empty()) {
                break;
            } else {
                // rule 0
                if (tokens[0].first == Token::literal || tokens[0].first==Token::str)
                    stack.emplace_back(new LiteralNode(tokens[0].second));
                else
                    stack.emplace_back(tokens[0].first);

                tokens.pop_front();
            }
        }

        if (stack.size() != 1 || stack[0].isTerminal) {
            std::cout << "Syntax error\n";
            exit(1);
        }

        return stack[0].data.nonTerminal;
    }
};
