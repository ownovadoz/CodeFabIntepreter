#pragma once

#include "../AssemblerUnit/Tokenizer/Value.h"

#include <memory>
#include <string>
#include <unordered_map>

using std::string;
using std::unordered_map;
using std::shared_ptr;

class Environment : public std::enable_shared_from_this<Environment> {
public:
    Environment() = default;
    explicit Environment(shared_ptr<Environment> enclosing);

    void define(const string& name, const Value& value);
    Value get(const string& name) const;
    void assign(const string& name, const Value& value);

private:
    shared_ptr<Environment> enclosing;
    unordered_map<string, Value> values;
};