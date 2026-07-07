#pragma once

#include <string>
#include <vector>

#include "Value.h"

using std::string;

enum class TokenType
{
    // 구분자 / 그룹핑
    LEFT_PAREN, RIGHT_PAREN, LEFT_BRACE, RIGHT_BRACE, SEMICOLON,

    // 연산자
    PLUS, MINUS, STAR, SLASH, BANG, EQUAL, GREATER, LESS,

    // 리터럴 / 식별자
    IDENTIFIER, STRING, NUMBER,

    // 키워드
    AND, ELSE, FALSE, FOR, IF, OR, PRINT, TRUE, VAR,

    END_OF_FILE
};

inline string tokenTypeToString(TokenType type)
{
    switch (type)
    {
    case TokenType::LEFT_PAREN:   return "LEFT_PAREN";
    case TokenType::RIGHT_PAREN:  return "RIGHT_PAREN";
    case TokenType::LEFT_BRACE:   return "LEFT_BRACE";
    case TokenType::RIGHT_BRACE:  return "RIGHT_BRACE";
    case TokenType::SEMICOLON:    return "SEMICOLON";
    case TokenType::PLUS:         return "PLUS";
    case TokenType::MINUS:        return "MINUS";
    case TokenType::STAR:         return "STAR";
    case TokenType::SLASH:        return "SLASH";
    case TokenType::BANG:         return "BANG";
    case TokenType::EQUAL:        return "EQUAL";
    case TokenType::GREATER:      return "GREATER";
    case TokenType::LESS:         return "LESS";
    case TokenType::IDENTIFIER:   return "IDENTIFIER";
    case TokenType::STRING:       return "STRING";
    case TokenType::NUMBER:       return "NUMBER";
    case TokenType::AND:          return "AND";
    case TokenType::ELSE:         return "ELSE";
    case TokenType::FALSE:        return "FALSE";
    case TokenType::FOR:          return "FOR";
    case TokenType::IF:           return "IF";
    case TokenType::OR:           return "OR";
    case TokenType::PRINT:        return "PRINT";
    case TokenType::TRUE:         return "TRUE";
    case TokenType::VAR:          return "VAR";
    case TokenType::END_OF_FILE:  return "EOF";
    default:                      return "UNKNOWN";
    }
}

struct Token
{
public:
    Token(TokenType type, string lexeme, Value literal, int line)
        : type(type), lexeme(std::move(lexeme)), literal(std::move(literal)), line(line)
    {
    }

    TokenType  getType()    const { return type; }
    string     getLexeme()  const { return lexeme; }
    Value      getLiteral() const { return literal; }
    int        getLine()    const { return line; }

    string toString() const
    {
        string result = "Token(" + tokenTypeToString(type) + ", \"" + lexeme + "\"";

        if (std::holds_alternative<double>(literal))
        {
            result += ", value=" + numberToString(std::get<double>(literal));
        }
        else if (std::holds_alternative<string>(literal))
        {
            result += ", value=\"" + std::get<string>(literal) + "\"";
        }

        result += ", line=" + std::to_string(line);
        result += ")";
        return result;
    }

private:
    TokenType type;
    string    lexeme;
    Value     literal;
    int       line;
};

inline string tokensToString(const std::vector<Token>& tokens)
{
    string result = "[ \n";
    for (int i = 0; i < static_cast<int>(tokens.size()); i++)
    {
        result += tokens[i].toString();
        if (i < static_cast<int>(tokens.size()) - 1)
            result += ", ";
    }
    result += "\n]";
    return result;
}
