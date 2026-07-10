#pragma once

#include "Value.h"

#include <string>
#include <vector>

using std::string;
using std::vector;

#define TOKEN_LIST_OPEN  "[ \n"
#define TOKEN_LIST_CLOSE "\n]"

#undef TRUE
#undef FALSE

enum class TokenType
{
    // 구분자 / 그룹핑
    LEFT_PAREN, RIGHT_PAREN, LEFT_BRACE, RIGHT_BRACE, LEFT_BRACKET, RIGHT_BRACKET, SEMICOLON, COMMA, DOT, COLON,

    // 연산자
    PLUS, MINUS, STAR, SLASH,
    BANG, BANG_EQUAL,
    EQUAL, EQUAL_EQUAL,
    GREATER, GREATER_EQUAL,
    LESS, LESS_EQUAL,

    // 리터럴 / 식별자
    IDENTIFIER, STRING, NUMBER,

    // 키워드
    AND, ARRAY, ELSE, FALSE, FOR, FUNC, IF, OR, PRINT, RETURN, TRUE, VAR,
    CLASS, THIS, SUPER, INSTANCEOF, IMPORT, ALIAS,

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
    case TokenType::LEFT_BRACKET:  return "LEFT_BRACKET";
    case TokenType::RIGHT_BRACKET: return "RIGHT_BRACKET";
    case TokenType::SEMICOLON:    return "SEMICOLON";
    case TokenType::COMMA:        return "COMMA";
    case TokenType::DOT:          return "DOT";
    case TokenType::COLON:        return "COLON";
    case TokenType::PLUS:         return "PLUS";
    case TokenType::MINUS:        return "MINUS";
    case TokenType::STAR:         return "STAR";
    case TokenType::SLASH:        return "SLASH";
    case TokenType::BANG:          return "BANG";
    case TokenType::BANG_EQUAL:    return "BANG_EQUAL";
    case TokenType::EQUAL:         return "EQUAL";
    case TokenType::EQUAL_EQUAL:   return "EQUAL_EQUAL";
    case TokenType::GREATER:       return "GREATER";
    case TokenType::GREATER_EQUAL: return "GREATER_EQUAL";
    case TokenType::LESS:          return "LESS";
    case TokenType::LESS_EQUAL:    return "LESS_EQUAL";
    case TokenType::IDENTIFIER:   return "IDENTIFIER";
    case TokenType::STRING:       return "STRING";
    case TokenType::NUMBER:       return "NUMBER";
    case TokenType::AND:          return "AND";
    case TokenType::ARRAY:        return "ARRAY";
    case TokenType::ELSE:         return "ELSE";
    case TokenType::FALSE:        return "FALSE";
    case TokenType::FOR:          return "FOR";
    case TokenType::FUNC:         return "FUNC";
    case TokenType::IF:           return "IF";
    case TokenType::OR:           return "OR";
    case TokenType::PRINT:        return "PRINT";
    case TokenType::RETURN:       return "RETURN";
    case TokenType::TRUE:         return "TRUE";
    case TokenType::VAR:          return "VAR";
    case TokenType::CLASS:        return "CLASS";
    case TokenType::THIS:         return "THIS";
    case TokenType::SUPER:        return "SUPER";
    case TokenType::INSTANCEOF:   return "INSTANCEOF";
    case TokenType::IMPORT:       return "IMPORT";
    case TokenType::ALIAS:        return "ALIAS";
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

        if (std::holds_alternative<bool>(literal))
        {
            result += ", value=" + string(std::get<bool>(literal) ? "true" : "false");
        }
        else if (std::holds_alternative<double>(literal))
        {
            result += ", value=" + numberToString(std::get<double>(literal));
        }
        else if (std::holds_alternative<string>(literal))
        {
            result += ", value=\"" + std::get<string>(literal) + "\"";
        }

        result += ", line=" + std::to_string(line) + ")";
        return result;
    }

private:
    TokenType type;
    string    lexeme;
    Value     literal;
    int       line;
};

inline string tokensToString(const vector<Token>& tokens)
{
    string result = TOKEN_LIST_OPEN;
    for (int i = 0; i < static_cast<int>(tokens.size()); i++)
    {
        result += tokens[i].toString();
        if (i < static_cast<int>(tokens.size()) - 1)
            result += ", ";
    }
    result += TOKEN_LIST_CLOSE;
    return result;
}
