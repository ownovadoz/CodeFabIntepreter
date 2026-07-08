#pragma once

#include "../AssemblerUnit/Parser/Statement.h"

#include <set>
#include <string>
#include <vector>

using std::set;
using std::string;
using std::vector;

class Checker
{
public:
    void enterScope();
    void exitScope();
    void declareVariable(const Token& name, const vector<string>& initializer_references);

    void check(Statement* root);

private:
    void checkStatement(Statement* stmt);
    void checkBlockStmt(BlockStmt* block);
    void checkVarDeclareStmt(VarDeclareStmt* var_decl);

    bool isDeclaredInCurrentScope(const string& name) const;

    vector<set<string>> scope_stack;
};
