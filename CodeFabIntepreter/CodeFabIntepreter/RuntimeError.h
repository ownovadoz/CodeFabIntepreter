#pragma once

#include "./AssemblerUnit//Tokenizer/Token.h"

#include <stdexcept>
#include <string>

class RuntimeError : public std::runtime_error {
    public:
        RuntimeError(std::string var_name, const std::string& message)
            : std::runtime_error(message), var_name(std::move(var_name))
        {
        }

        std::string var_name;
};