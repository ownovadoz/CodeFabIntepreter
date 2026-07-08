#include "Environment.h"
#include "../CodeFabException.h"

Environment::Environment(Environment* enclosing)
    : enclosing(enclosing)
{}

void Environment::define(const string& name, const Value& value)
{
    values[name] = value;
}

Value Environment::get(const string& name) const
{
    auto found = values.find(name);
    if (found != values.end()) return found->second;

    throw CodeFabException(0, "Undefined variable '" + name + "'.");
}

void Environment::assign(const string& name, const Value& value)
{
    auto found = values.find(name);
    if (found != values.end()) {
        found->second = value;
        return;
    }
    throw CodeFabException(0, "Undefined variable '" + name + "'.");
}