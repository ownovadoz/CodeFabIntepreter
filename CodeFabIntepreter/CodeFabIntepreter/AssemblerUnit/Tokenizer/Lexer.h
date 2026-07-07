#pragma once

#include "Token.h"

#include <string>
#include <vector>

using std::string;
using std::vector;

class Lexer
{
public:
    explicit Lexer(string source);

    vector<Token> scanTokens();

private:
    void scanToken();

    bool match(char expected);
    char advance();
    char peek() const;
    char peekNext() const;
    bool isAtEnd() const;
    void scanString();
    void scanNumber();
    void addToken(TokenType type);
    void addToken(TokenType type, Value literal);

    string       source;
    vector<Token> tokens;
    int start   = 0;
    int current = 0;
    int line    = 1;
};
