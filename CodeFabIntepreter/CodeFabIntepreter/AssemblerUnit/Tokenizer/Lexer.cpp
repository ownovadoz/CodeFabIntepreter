#include "Lexer.h"

#include <stdexcept>
#include <unordered_map>

using std::isdigit;
using std::isalpha;
using std::isalnum;
using std::unordered_map;
using std::string_view;

Lexer::Lexer(string source) : source(std::move(source))
{
}

vector<Token> Lexer::scanTokens()
{
    while (!isAtEnd())
    {
        start = current;
        scanToken();
    }

    tokens.emplace_back(TokenType::END_OF_FILE, "", std::monostate{}, line);
    return tokens;
}

void Lexer::scanToken()
{
    char current_char = advance();
    switch (current_char)
    {
        case '(': addToken(TokenType::LEFT_PAREN);  break;
        case ')': addToken(TokenType::RIGHT_PAREN); break;
        case '{': addToken(TokenType::LEFT_BRACE);  break;
        case '}': addToken(TokenType::RIGHT_BRACE); break;
        case ';': addToken(TokenType::SEMICOLON);   break;
        case '[': addToken(TokenType::LEFT_BRACKET);  break;
        case ']': addToken(TokenType::RIGHT_BRACKET); break;
        case '+': addToken(TokenType::PLUS);        break;
        case '-': addToken(TokenType::MINUS);       break;
        case '*': addToken(TokenType::STAR);        break;
        case '!': addToken(match('=') ? TokenType::BANG_EQUAL    : TokenType::BANG);    break;
        case '=': addToken(match('=') ? TokenType::EQUAL_EQUAL   : TokenType::EQUAL);   break;
        case '>': addToken(match('=') ? TokenType::GREATER_EQUAL : TokenType::GREATER); break;
        case '<': addToken(match('=') ? TokenType::LESS_EQUAL    : TokenType::LESS);    break;


        case '/':
            if (match('/'))
            {
                while (peek() != '\n' && !isAtEnd()) advance();
            }
            else
            {
                addToken(TokenType::SLASH);
            }
            break;

        case ' ':
        case '\r':
        case '\t':
            break;

        case '\n':
            line++;
            break;

        case '"': scanString(); break;

        default:
            if (isdigit(static_cast<unsigned char>(current_char)))
                scanNumber();
            else if (isalpha(static_cast<unsigned char>(current_char)) || current_char == '_')
                scanIdentifier();
            break;
    }
}

bool Lexer::match(char expected)
{
    if (isAtEnd() || source[current] != expected) return false;
    current++;
    return true;
}

char Lexer::advance()
{
    return source[current++];
}

char Lexer::peek() const
{
    if (isAtEnd()) return '\0';
    return source[current];
}

bool Lexer::isAtEnd() const
{
    return current >= static_cast<int>(source.size());
}

void Lexer::addToken(TokenType type)
{
    addToken(type, std::monostate{});
}

void Lexer::addToken(TokenType type, Value literal)
{
    string text = source.substr(start, current - start);
    tokens.emplace_back(type, std::move(text), std::move(literal), line);
}

char Lexer::peekNext() const
{
    if (current + 1 >= static_cast<int>(source.size())) return '\0';
    return source[current + 1];
}

void Lexer::scanString()
{
    while (peek() != '"' && !isAtEnd())
    {
        if (peek() == '\n') line++;
        advance();
    }

    advance(); // 닫는 "

    string value = source.substr(start + 1, current - start - 2);
    addToken(TokenType::STRING, value);
}

void Lexer::scanIdentifier()
{
    while (isalnum(static_cast<unsigned char>(peek())) || peek() == '_')
        advance();

    static const unordered_map<string_view, TokenType> keywords = {
        {"and",   TokenType::AND},
        {"Array", TokenType::ARRAY},
        {"else",  TokenType::ELSE},
        {"false", TokenType::FALSE},
        {"for",   TokenType::FOR},
        {"if",    TokenType::IF},
        {"or",    TokenType::OR},
        {"print", TokenType::PRINT},
        {"true",  TokenType::TRUE},
        {"var",   TokenType::VAR},
    };

    string_view text(source.data() + start, current - start);
    auto found = keywords.find(text);
    TokenType type = (found != keywords.end()) ? found->second : TokenType::IDENTIFIER;

    if (type == TokenType::TRUE)       addToken(type, true);
    else if (type == TokenType::FALSE) addToken(type, false);
    else                               addToken(type);
}

void Lexer::scanNumber()
{
    while (isdigit(static_cast<unsigned char>(peek()))) advance();

    if (peek() == '.' && isdigit(static_cast<unsigned char>(peekNext())))
    {
        advance(); // .
        while (isdigit(static_cast<unsigned char>(peek()))) advance();
    }

    double value = std::stod(source.substr(start, current - start));
    addToken(TokenType::NUMBER, value);
}
