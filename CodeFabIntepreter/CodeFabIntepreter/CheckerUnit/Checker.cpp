#include "Checker.h"

#include <algorithm>

void Checker::enterScope()
{
    scope_stack.emplace_back();
}

void Checker::exitScope()
{
    if (scope_stack.empty()) return;

    scope_stack.pop_back();
}

void Checker::declareVariable(const Token& name, const vector<string>& initializer_references)
{
    scope_stack.back().insert(name.getLexeme());
}

bool Checker::isDeclaredInCurrentScope(const string& name) const
{
    if (scope_stack.empty()) return false;

    return scope_stack.back().count(name) > 0;
}

void Checker::check(Statement* root)
{
    enterScope();
    checkStatement(root);
}

void Checker::checkStatement(Statement* stmt)
{
    if (stmt == nullptr) return;

    if (BlockStmt* block = dynamic_cast<BlockStmt*>(stmt))
    {
        checkBlockStmt(block);
        return;
    }

    if (VarDeclareStmt* var_decl = dynamic_cast<VarDeclareStmt*>(stmt))
    {
        checkVarDeclareStmt(var_decl);
        return;
    }
}

void Checker::checkBlockStmt(BlockStmt* block)
{
    enterScope();

    for (Statement* stmt : block->getStatements())
        checkStatement(stmt);

    exitScope();
}

void Checker::checkVarDeclareStmt(VarDeclareStmt* var_decl)
{
    declareVariable(var_decl->getName(), {});
}
