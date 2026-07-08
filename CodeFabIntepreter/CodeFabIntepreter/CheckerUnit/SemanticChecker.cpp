#include "SemanticChecker.h"

CheckResult SemanticChecker::check(Statement* root)
{
    return CheckResult::isOK();
}

CheckResult SemanticChecker::checkStatement(Statement* stmt)
{
    return CheckResult::isOK();
}

CheckResult SemanticChecker::checkBlockStmt(BlockStmt* block)
{
    return CheckResult::isOK();
}

CheckResult SemanticChecker::checkVarDeclareStmt(VarDeclareStmt* var_decl)
{
    return CheckResult::isOK();
}
