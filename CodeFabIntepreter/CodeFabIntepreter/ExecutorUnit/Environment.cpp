#include "Environment.h"

Environment::Environment(Environment* enclosing)
    : enclosing(enclosing)
{}

void Environment::define(const string& name, const Value& value)
{}

optional<Value> Environment::get(const string& name) const
{
    return std::nullopt;
}

bool Environment::assign(const string& name, const Value& value)
{
    return true;
}