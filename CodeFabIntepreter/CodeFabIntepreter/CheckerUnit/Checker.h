#pragma once

#include "../AssemblerUnit/Parser/Statement.h"
#include "../InterfaceForCodeFabTest.h"
#include "../Visitor.h"

#include <string>
#include <unordered_map>
#include <vector>

using std::string;
using std::unordered_map;
using std::vector;

#ifdef _DEBUG
class Checker : public IChecker, public ExprVisitor, public StmtVisitor
#else
class Checker : public ExprVisitor, public StmtVisitor
#endif
{
public:
    Checker();

    class ScopeGuard
    {
    public:
        explicit ScopeGuard(Checker& checker) : checker(checker) { checker.enterScope(); }
        ~ScopeGuard() { checker.exitScope(); }

        ScopeGuard(const ScopeGuard&) = delete;
        ScopeGuard& operator=(const ScopeGuard&) = delete;

    private:
        Checker& checker;
    };

#ifdef _DEBUG
    void check(Statement* root) override;
#else
    void check(Statement* root);
#endif

    void visitLiteralExpr(const LiteralExpr& expr) override;
    void visitVariableExpr(const VariableExpr& expr) override;
    void visitAssignExpr(const AssignExpr& expr) override;
    void visitBinaryExpr(const BinaryExpr& expr) override;
    void visitUnaryExpr(const UnaryExpr& expr) override;
    void visitGroupingExpr(const GroupingExpr& expr) override;
    void visitLogicalExpr(const LogicalExpr& expr) override;

    void visitExpressionStmt(const ExpressionStmt& stmt) override;
    void visitIfStmt(const IfStmt& stmt) override;
    void visitBlockStmt(const BlockStmt& stmt) override;
    void visitVarDeclareStmt(const VarDeclareStmt& stmt) override;
    void visitPrintStmt(const PrintStmt& stmt) override;
    void visitForStmt(const ForStmt& stmt) override;

private:
    void enterScope();
    void exitScope();

    void declare(const Token& name);
    void define(const Token& name);

    void checkStatement(const Statement* stmt);
    void checkStatementInNewScope(const Statement* stmt);
    void checkExpression(const Expression* expr);

    // 스코프 내 각 변수 이름은 declare 시 false(선언됨, 아직 정의되지 않음)로,
    // define 시 true(정의 완료)로 표시된다. 초기화식을 검사하는 시점에는
    // 아직 정의되지 않은 상태이므로, 이 시점에 같은 스코프에서 자신의 이름이
    // false로 발견되면 자기참조로 판단한다.
    vector<unordered_map<string, bool>> scope_stack;
};
