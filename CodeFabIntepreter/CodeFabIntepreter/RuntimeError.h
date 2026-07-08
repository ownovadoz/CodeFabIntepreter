#pragma once

#include <stdexcept>
#include <string>

using std::string;
using std::runtime_error;
using std::move;

class RuntimeError : public runtime_error {
public:
        RuntimeError(string var_name, const string& message)
            : runtime_error(message), var_name(move(var_name)) {
        }

        string getVarName()
        {
            return var_name;
        }
private:
        std::string var_name;
};