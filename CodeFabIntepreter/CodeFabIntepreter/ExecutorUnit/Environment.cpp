#include "Environment.h"
#include "../RuntimeError.h"

Environment::Environment(Environment* enclosing)
    : enclosing(enclosing)
{}

void Environment::define(const string& name, const Value& value)
{
    values[name] = value;
}

optional<Value> Environment::get(const string& name) const
{
    auto found = values.find(name);
    if (found != values.end())
    {
        return found->second;
    }

    throw RuntimeError(name, "Undefined variable '" + name + "'.");
}

bool Environment::assign(const string& name, const Value& value)
{
    auto found = values.find(name);
    if (found != values.end())
    {
        found->second = value;
        return true;
    }
    throw RuntimeError(name, "Undefined variable '" + name + "'.");
}