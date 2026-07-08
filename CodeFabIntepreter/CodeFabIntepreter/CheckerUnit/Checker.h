#pragma once

#include "../AssemblerUnit/Parser/Statement.h"

#include <set>
#include <string>
#include <vector>

using std::set;
using std::string;
using std::vector;

class CheckResult
{
public:
    static CheckResult isOK();
    static CheckResult checkerUnitHasError(string message);

    bool hasError = false;
    string message;
};

class Checker
{
public:
    void enterScope();
    void exitScope();
    CheckResult declareVariable(const string& name, const vector<string>& initializer_references);

    CheckResult check(Statement* root);

private:
    CheckResult checkStatement(Statement* stmt);
    CheckResult checkBlockStmt(BlockStmt* block);
    CheckResult checkVarDeclareStmt(VarDeclareStmt* var_decl);

    bool isDeclaredInCurrentScope(const string& name) const;

    vector<set<string>> scope_stack;
};
