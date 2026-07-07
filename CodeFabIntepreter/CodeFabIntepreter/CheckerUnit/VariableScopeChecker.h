#pragma once

#include <set>
#include <string>
#include <vector>

using std::set;
using std::vector;
using std::string;

struct CheckResult
{
public:
    static CheckResult isOK();
    static CheckResult checkerUnitHasError(string message);

    bool hasError = false;
    string message;
};

class VariableScopeChecker
{
public:
    void enterScope();
    void exitScope();
    CheckResult declareVariable(const string& name, const vector<string>& initializer_references);

private:
    bool isDeclaredInCurrentScope(const string& name) const;

    vector<set<string>> scope_stack;
};
