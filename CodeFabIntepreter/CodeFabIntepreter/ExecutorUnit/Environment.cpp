#include "Environment.h"
#include "../CodeFabException.h"

#include <memory>

using std::make_shared;

Environment::Environment(shared_ptr<Environment> enclosing) : enclosing(std::move(enclosing))
{
}


void Environment::define(const string& name, const Value& value)
{
    values[name] = value;
}

Value Environment::get(const string& name) const
{
    auto found = values.find(name);
    if (found != values.end()) return found->second;

    if (enclosing) return enclosing->get(name);

    throw CodeFabException(0, "Undefined variable '" + name + "'.");
}

void Environment::assign(const string& name, const Value& value)
{
    auto found = values.find(name);
    if (found != values.end()) {
        found->second = value;
        return;
    }

    if (enclosing) {
        enclosing->assign(name, value);
        return;
    }

    throw CodeFabException(0, "Undefined variable '" + name + "'.");
}