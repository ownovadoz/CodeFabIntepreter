#pragma once
#include "AssemblerUnit/Tokenizer/Token.h"

#include <stdexcept>
#include <string>

using std::string;

class CodeFabException : public std::runtime_error {
public:
    CodeFabException(int line, const string& message)
        : std::runtime_error(formatMessage(line, message)), line(line), message(message) {
    }

    CodeFabException(const Token& token, const string& message)
        : std::runtime_error(formatMessage(token, message)), line(token.getLine()), message(message) {
    }

    int getLine() const { return line; }
    const string& getMessage() const { return message; }

private:
    static string linePrefix(int line) {
        return "[line " + std::to_string(line) + "] ";
    }
    static string formatMessage(int line, const string& message) {
        return linePrefix(line) + "Error: " + message;
    }
    static string formatMessage(const Token& token, const string& message) {
        const int token_line = token.getLine();
        if (token.getType() == TokenType::END_OF_FILE) return linePrefix(token_line) + "Error at end: " + message;

        return linePrefix(token_line)
            + "Type : " + tokenTypeToString(token.getType())
            + ", Lexeme : " + token.getLexeme()
            + ", literal : " + stringify(token.getLiteral())
            + ", message : " + message;
    }
    int line;
    string message;
};

