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

    for (Statement* stmt : block->getStatements())
    {
        CheckResult result = checkStatement(stmt);
        if (result.hasError)
        {
            scope_checker.exitScope();
            return result;
        }
    }

    scope_checker.exitScope();
    return CheckResult::isOK();
}

CheckResult SemanticChecker::checkVarDeclareStmt(VarDeclareStmt* var_decl)
{
    return scope_checker.declareVariable(var_decl->getName().getLexeme(), {});
}
