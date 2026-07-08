#include "SemanticChecker.h"

CheckResult SemanticChecker::check(Statement* root)
{
    scope_checker.enterScope();
    return checkStatement(root);
}

CheckResult SemanticChecker::checkStatement(Statement* stmt)
{
    if (stmt == nullptr) return CheckResult::isOK();

    if (BlockStmt* block = dynamic_cast<BlockStmt*>(stmt))
        return checkBlockStmt(block);

    if (VarDeclareStmt* var_decl = dynamic_cast<VarDeclareStmt*>(stmt))
        return checkVarDeclareStmt(var_decl);

    return CheckResult::isOK();
}

CheckResult SemanticChecker::checkBlockStmt(BlockStmt* block)
{
    scope_checker.enterScope();

    CheckResult result = CheckResult::isOK();
    for (Statement* stmt : block->getStatements())
    {
        result = checkStatement(stmt);
        if (result.hasError) break;
    }

    scope_checker.exitScope();
    return result;
}

CheckResult SemanticChecker::checkVarDeclareStmt(VarDeclareStmt* var_decl)
{
    return scope_checker.declareVariable(var_decl->getName().getLexeme(), {});
}
