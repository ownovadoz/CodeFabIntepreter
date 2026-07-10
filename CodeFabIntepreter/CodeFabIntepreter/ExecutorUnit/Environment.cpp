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

Value Environment::get(const Token& name) const
{
    auto found = values.find(name.getLexeme());
    if (found != values.end()) return found->second;

    if (enclosing) return enclosing->get(name);

    throw CodeFabException(name, "Undefined variable '" + name.getLexeme() + "'.");
}

void Environment::assign(const Token& name, const Value& value)
{
    auto found = values.find(name.getLexeme());
    if (found != values.end()) {
        found->second = value;
        return;
    }

    if (enclosing) {
        enclosing->assign(name, value);
        return;
    }

    throw CodeFabException(name, "Undefined variable '" + name.getLexeme() + "'.");
}

Environment* Environment::ancestor(int distance)
{
    Environment* environment = this;
    for (int i = 0; i < distance; i++)
        environment = environment->enclosing.get();

    return environment;
}

Value Environment::getAt(int distance, const string& name)
{
    return ancestor(distance)->values.at(name);
}

void Environment::assignAt(int distance, const string& name, const Value& value)
{
    ancestor(distance)->values[name] = value;
}