#include "CodeFabClass.h"

#include "CodeFabInstance.h"

#include <memory>

using std::const_pointer_cast;
using std::make_shared;

CodeFabClass::CodeFabClass(string name, shared_ptr<CodeFabClass> superclass, unordered_map<string, shared_ptr<CodeFabFunction>> methods)
    : name{ std::move(name) }, superclass{ std::move(superclass) }, methods{ std::move(methods) }
{
}

const string& CodeFabClass::getName() const
{
    return name;
}

const shared_ptr<CodeFabClass>& CodeFabClass::getSuperclass() const
{
    return superclass;
}

shared_ptr<CodeFabFunction> CodeFabClass::findMethod(const string& method_name) const
{
    auto found = methods.find(method_name);
    if (found != methods.end()) return found->second;

    if (superclass) return superclass->findMethod(method_name);

    return nullptr;
}

bool CodeFabClass::isSubclassOf(const CodeFabClass* other) const
{
    for (const CodeFabClass* current = this; current != nullptr; current = current->superclass.get())
        if (current == other) return true;

    return false;
}

int CodeFabClass::arity() const
{
    shared_ptr<CodeFabFunction> initializer = findMethod("init");
    return initializer ? initializer->arity() : 0;
}

Value CodeFabClass::call(Interpreter& interpreter, const vector<Value>& arguments) const
{
    auto instance = make_shared<CodeFabInstance>(const_pointer_cast<CodeFabClass>(shared_from_this()));

    shared_ptr<CodeFabFunction> initializer = findMethod("init");
    if (initializer) initializer->bind(instance)->call(interpreter, arguments);

    return instance;
}

string CodeFabClass::toString() const
{
    return name;
}
