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

    int getLine() const { return line; }
    const string& getMessage() const { return message; }

private:
    static string formatMessage(int line, const string& message) {
        return "[line " + std::to_string(line) + "] Error: " + message;
    }
    int line;
    string message;
};

