#include "Checker.h"

#include "../CodeFabException.h"

Checker::Checker()
{
    // 전역 스코프는 Checker 인스턴스가 살아있는 동안 유지되어, PromptShell처럼
    // 한 줄씩 나뉘어 들어오는 여러 번의 check() 호출에서도 전역 변수의 중복
    // 선언을 검출할 수 있게 한다 (Interpreter의 global_environment와 대응).
    beginScope();
}

void Checker::beginScope()
{
    scope_stack.emplace_back();
}

void Checker::endScope()
{
    if (scope_stack.empty()) return;

    scope_stack.pop_back();
}

void Checker::declare(const Token& name)
{
    if (scope_stack.empty()) return;

    unordered_map<string, bool>& scope = scope_stack.back();
    const string lexeme = name.getLexeme();

    if (scope.find(lexeme) != scope.end())
        throw CodeFabException(name, "이미 해당 변수는 현재 스코프에서 사용중입니다: '" + lexeme + "'");

    scope[lexeme] = false;
}

void Checker::define(const Token& name)
{
    if (scope_stack.empty()) return;

    scope_stack.back()[name.getLexeme()] = true;
}

void Checker::check(Statement* root)
{
    resolveStmt(root);
}

void Checker::resolveStmt(const Statement* stmt)
{
    if (stmt == nullptr) return;

    stmt->accept(*this);
}

void Checker::visitExpressionStmt(const ExpressionStmt& stmt)
{
    resolveExpr(stmt.getExpr());
}

void Checker::visitIfStmt(const IfStmt& stmt)
{
    resolveExpr(stmt.getCondition());

    // 분기별로 독립된 스코프를 부여해, 서로 배타적인 then/else 분기에서
    // 같은 이름을 선언해도 중복 선언으로 오검출되지 않도록 한다.
    resolveStmtInNewScope(stmt.getThenBranch());
    resolveStmtInNewScope(stmt.getElseBranch());
}

void Checker::resolveStmtInNewScope(const Statement* stmt)
{
    ScopeGuard guard(*this);
    resolveStmt(stmt);
}

void Checker::visitBlockStmt(const BlockStmt& stmt)
{
    ScopeGuard guard(*this);

    for (const auto& child : stmt.getStatements())
        resolveStmt(child.get());
}

void Checker::visitVarDeclareStmt(const VarDeclareStmt& stmt)
{
    declare(stmt.getName());
    resolveExpr(stmt.getInitializer());
    define(stmt.getName());
}

void Checker::visitPrintStmt(const PrintStmt& stmt)
{
    resolveExpr(stmt.getExpr());
}

void Checker::visitForStmt(const ForStmt& stmt)
{
    // for 문의 초기화 변수는 for 문 자신의 스코프에 속하며, body 블록과는
    // 별개로 for 문이 끝나면 함께 소멸한다.
    ScopeGuard guard(*this);

    resolveStmt(stmt.getInit());
    resolveExpr(stmt.getCondition());
    resolveExpr(stmt.getIncrement());
    resolveStmt(stmt.getBody());
}

void Checker::resolveExpr(const Expression* expr)
{
    if (expr == nullptr) return;

    expr->accept(*this);
}

void Checker::visitLiteralExpr(const LiteralExpr&)
{
}

void Checker::visitVariableExpr(const VariableExpr& expr)
{
    if (scope_stack.empty()) return;

    const unordered_map<string, bool>& scope = scope_stack.back();
    auto found = scope.find(expr.getToken().getLexeme());

    if (found != scope.end() && found->second == false)
        throw CodeFabException(expr.getToken(), "자신의 초기화식에서 지역변수를 읽을 수 없습니다: '" + expr.getToken().getLexeme() + "'");
}

void Checker::visitAssignExpr(const AssignExpr& expr)
{
    resolveExpr(expr.getValue());
}

void Checker::visitBinaryExpr(const BinaryExpr& expr)
{
    resolveExpr(expr.getLeft());
    resolveExpr(expr.getRight());
}

void Checker::visitUnaryExpr(const UnaryExpr& expr)
{
    resolveExpr(expr.getExpr());
}

void Checker::visitGroupingExpr(const GroupingExpr& expr)
{
    resolveExpr(expr.getExpr());
}

void Checker::visitLogicalExpr(const LogicalExpr& expr)
{
    resolveExpr(expr.getLeft());
    resolveExpr(expr.getRight());
}
