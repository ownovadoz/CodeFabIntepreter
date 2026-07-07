#include "Lexer.h"

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
    char c = advance();
    switch (c)
    {
        case '(': addToken(TokenType::LEFT_PAREN);  break;
        case ')': addToken(TokenType::RIGHT_PAREN); break;
        case '{': addToken(TokenType::LEFT_BRACE);  break;
        case '}': addToken(TokenType::RIGHT_BRACE); break;
        case ';': addToken(TokenType::SEMICOLON);   break;
        case '+': addToken(TokenType::PLUS);        break;
        case '-': addToken(TokenType::MINUS);       break;
        case '*': addToken(TokenType::STAR);        break;
        case '!': addToken(TokenType::BANG);        break;
        case '=': addToken(TokenType::EQUAL);       break;
        case '>': addToken(TokenType::GREATER);     break;
        case '<': addToken(TokenType::LESS);        break;

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

        default:
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
