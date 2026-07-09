#pragma once

#include "../AssemblerUnit/Parser/Statement.h"
#include "../InterfaceForCodeFabTest.h"
#include "../Visitor.h"

#include <set>
#include <string>
#include <vector>

using std::set;
using std::string;
using std::vector;

#ifdef _DEBUG
class Checker : public IChecker, public ExprVisitor, public StmtVisitor
#else
class Checker : public ExprVisitor, public StmtVisitor
#endif
{
public:
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

    void declareVariable(const Token& name, const vector<string>& initializer_references);

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

    void checkStatement(const Statement* stmt);
    void checkStatementInNewScope(const Statement* stmt);
    void checkExpression(const Expression* expr);
    vector<string> collectIdentifierReferences(const Expression* expr);

    bool isDeclaredInCurrentScope(const string& name) const;

    vector<set<string>> scope_stack;
    vector<string>* collecting_references = nullptr;
};
