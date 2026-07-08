#include "Checker.h"

#include "../CodeFabException.h"

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
    const string lexeme = name.getLexeme();

    if (isDeclaredInCurrentScope(lexeme))
        throw CodeFabException(name, "이미 해당 변수는 현재 스코프에서 사용중입니다: '" + lexeme + "'");

    bool is_self_referenced = std::find(initializer_references.begin(), initializer_references.end(), lexeme) != initializer_references.end();
    if (is_self_referenced)
        throw CodeFabException(name, "자신의 초기화식에서 지역변수를 읽을 수 없습니다: '" + lexeme + "'");

    scope_stack.back().insert(lexeme);
}

bool Checker::isDeclaredInCurrentScope(const string& name) const
{
    if (scope_stack.empty()) return false;

    return scope_stack.back().count(name) > 0;
}

void Checker::check(Statement* root)
{
    ScopeGuard guard(*this);
    checkStatement(root);
}

void Checker::checkStatement(Statement* stmt)
{
    if (stmt) stmt->accept(*this);
}

void Checker::visitBlockStmt(BlockStmt& stmt)
{
    ScopeGuard guard(*this);

    for (const auto& child : stmt.getStatements())
        checkStatement(child.get());
}

void Checker::visitVarDeclareStmt(VarDeclareStmt& stmt)
{
    declareVariable(stmt.getName(), {});
}

void Checker::visitExpressionStmt(ExpressionStmt& stmt)
{
}

void Checker::visitIfStmt(IfStmt& stmt)
{
}

void Checker::visitPrintStmt(PrintStmt& stmt)
{
}

void Checker::visitForStmt(ForStmt& stmt)
{
}
