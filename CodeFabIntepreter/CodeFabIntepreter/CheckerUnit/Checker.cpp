#include "Checker.h"

#include <algorithm>

CheckResult CheckResult::isOK()
{
    return CheckResult{ false, "" };
}

CheckResult CheckResult::checkerUnitHasError(string message)
{
    return CheckResult{ true, std::move(message) };
}

void Checker::enterScope()
{
    scope_stack.emplace_back();
}

void Checker::exitScope()
{
    if (scope_stack.empty()) return;

    scope_stack.pop_back();
}

CheckResult Checker::declareVariable(const string& name, const vector<string>& initializer_references)
{
    if (isDeclaredInCurrentScope(name))
        return CheckResult::checkerUnitHasError("이미 해당 변수는 현재 스코프에서 사용중입니다: '" + name + "'");

    bool is_self_referenced = std::find(initializer_references.begin(), initializer_references.end(), name) != initializer_references.end();
    if (is_self_referenced)
        return CheckResult::checkerUnitHasError("자신의 초기화식에서 지역변수를 읽을 수 없습니다: '" + name + "'");

    scope_stack.back().insert(name);
    return CheckResult::isOK();
}

bool Checker::isDeclaredInCurrentScope(const string& name) const
{
    if (scope_stack.empty()) return false;

    return scope_stack.back().count(name) > 0;
}

CheckResult Checker::check(Statement* root)
{
    enterScope();
    return checkStatement(root);
}

CheckResult Checker::checkStatement(Statement* stmt)
{
    if (stmt == nullptr) return CheckResult::isOK();

    if (BlockStmt* block = dynamic_cast<BlockStmt*>(stmt))
        return checkBlockStmt(block);

    if (VarDeclareStmt* var_decl = dynamic_cast<VarDeclareStmt*>(stmt))
        return checkVarDeclareStmt(var_decl);

    return CheckResult::isOK();
}

CheckResult Checker::checkBlockStmt(BlockStmt* block)
{
    enterScope();

    CheckResult result = CheckResult::isOK();
    for (Statement* stmt : block->getStatements())
    {
        result = checkStatement(stmt);
        if (result.hasError) break;
    }

    exitScope();
    return result;
}

CheckResult Checker::checkVarDeclareStmt(VarDeclareStmt* var_decl)
{
    return declareVariable(var_decl->getName().getLexeme(), {});
}
