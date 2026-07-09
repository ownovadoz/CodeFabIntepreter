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
    ScopeGuard guard(*this);

    for (const auto& stmt : block->getStatements())
        checkStatement(stmt.get());
}

void Checker::checkVarDeclareStmt(VarDeclareStmt* var_decl)
{
    vector<string> references = collectIdentifierReferences(var_decl->getInitializer());
    declareVariable(var_decl->getName(), references);
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
