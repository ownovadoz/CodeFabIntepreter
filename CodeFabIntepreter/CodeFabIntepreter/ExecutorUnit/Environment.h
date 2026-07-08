#pragma once

#include "../AssemblerUnit/Tokenizer/Value.h"

#include <string>
#include <unordered_map>

using std::string;
using std::unordered_map;

class Environment {
public:
    explicit Environment(Environment* enclosing = nullptr);

    void define(const string& name, const Value& value);
    Value get(const string& name) const;
    void assign(const string& name, const Value& value);

private:
    Environment* enclosing;
    unordered_map<string, Value> values;
};