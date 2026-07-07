#pragma once

#include <set>
#include <string>
#include <vector>

struct CheckResult
{
public:
    static CheckResult isOK();
    static CheckResult checkerUnitHasError(std::string message);

    bool hasError = false;
    std::string message;
};

class VariableScopeChecker
{
public:
    void enterScope();
    void exitScope();
    CheckResult declareVariable(const std::string& name, const std::vector<std::string>& initializer_references);

private:
    bool isDeclaredInCurrentScope(const std::string& name) const;

    std::vector<std::set<std::string>> scope_stack;
};
