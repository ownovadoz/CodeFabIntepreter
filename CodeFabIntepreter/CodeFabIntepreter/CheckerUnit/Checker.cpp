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

void Checker::checkStatement(const Statement* stmt)
{
    if (stmt == nullptr) return;

    stmt->accept(*this);
}

void Checker::visitExpressionStmt(const ExpressionStmt& stmt)
{
    checkExpression(stmt.getExpr());
}

void Checker::visitIfStmt(const IfStmt& stmt)
{
    checkExpression(stmt.getCondition());

    // 분기별로 독립된 스코프를 부여해, 서로 배타적인 then/else 분기에서
    // 같은 이름을 선언해도 중복 선언으로 오검출되지 않도록 한다.
    checkStatementInNewScope(stmt.getThenBranch());
    checkStatementInNewScope(stmt.getElseBranch());
}

void Checker::checkStatementInNewScope(const Statement* stmt)
{
    ScopeGuard guard(*this);
    checkStatement(stmt);
}

void Checker::visitBlockStmt(const BlockStmt& stmt)
{
    ScopeGuard guard(*this);

    for (const auto& child : stmt.getStatements())
        checkStatement(child.get());
}

void Checker::visitVarDeclareStmt(const VarDeclareStmt& stmt)
{
    vector<string> references = collectIdentifierReferences(stmt.getInitializer());
    declareVariable(stmt.getName(), references);
}

void Checker::visitPrintStmt(const PrintStmt& stmt)
{
    checkExpression(stmt.getExpr());
}

void Checker::visitForStmt(const ForStmt& stmt)
{
    // for 문의 초기화 변수는 for 문 자신의 스코프에 속하며, body 블록과는
    // 별개로 for 문이 끝나면 함께 소멸한다.
    ScopeGuard guard(*this);

    checkStatement(stmt.getInit());
    checkExpression(stmt.getCondition());
    checkExpression(stmt.getIncrement());
    checkStatement(stmt.getBody());
}

void Checker::checkExpression(const Expression* expr)
{
    if (expr == nullptr) return;

    expr->accept(*this);
}

vector<string> Checker::collectIdentifierReferences(const Expression* expr)
{
    vector<string> references;
    vector<string>* previous_references = collecting_references;

    collecting_references = &references;
    checkExpression(expr);
    collecting_references = previous_references;

    return references;
}

void Checker::visitLiteralExpr(const LiteralExpr&)
{
}

void Checker::visitVariableExpr(const VariableExpr& expr)
{
    if (collecting_references != nullptr)
        collecting_references->push_back(expr.getToken().getLexeme());
}

void Checker::visitAssignExpr(const AssignExpr& expr)
{
    checkExpression(expr.getValue());
}

void Checker::visitBinaryExpr(const BinaryExpr& expr)
{
    checkExpression(expr.getLeft());
    checkExpression(expr.getRight());
}

void Checker::visitUnaryExpr(const UnaryExpr& expr)
{
    checkExpression(expr.getExpr());
}

void Checker::visitGroupingExpr(const GroupingExpr& expr)
{
    checkExpression(expr.getExpr());
}

void Checker::visitLogicalExpr(const LogicalExpr& expr)
{
    checkExpression(expr.getLeft());
    checkExpression(expr.getRight());
}
