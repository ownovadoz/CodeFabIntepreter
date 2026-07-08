#pragma once

#include "VariableScopeChecker.h"
#include "../AssemblerUnit/Parser/Statement.h"

class SemanticChecker
{
public:
    CheckResult check(Statement* root);

private:
    CheckResult checkStatement(Statement* stmt);
    CheckResult checkBlockStmt(BlockStmt* block);
    CheckResult checkVarDeclareStmt(VarDeclareStmt* var_decl);

    VariableScopeChecker scope_checker;
};
