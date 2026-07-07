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
    if (found != values.end()) return found->second;

    if (enclosing != nullptr) return enclosing->get(name);

    throw RuntimeError(name, "Undefined variable '" + name + "'.");
}

void Environment::assign(const string& name, const Value& value)
{
    auto found = values.find(name);
    if (found != values.end()) {
        found->second = value;
        return;
    }

    if (enclosing != nullptr) {
        enclosing->assign(name, value);
        return;
    }

    throw RuntimeError(name, "Undefined variable '" + name + "'.");
}