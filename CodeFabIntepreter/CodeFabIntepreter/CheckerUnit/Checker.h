#pragma once

#include "../AssemblerUnit/Parser/Statement.h"
#include "../Visitor.h"

#include <set>
#include <string>
#include <vector>

using std::set;
using std::string;
using std::vector;

class Checker : public StmtVisitor
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

    void check(Statement* root);

private:
    void enterScope();
    void exitScope();

    void checkStatement(Statement* stmt);

    void visitExpressionStmt(ExpressionStmt& stmt) override;
    void visitIfStmt(IfStmt& stmt) override;
    void visitBlockStmt(BlockStmt& stmt) override;
    void visitVarDeclareStmt(VarDeclareStmt& stmt) override;
    void visitPrintStmt(PrintStmt& stmt) override;
    void visitForStmt(ForStmt& stmt) override;

    bool isDeclaredInCurrentScope(const string& name) const;

    vector<set<string>> scope_stack;
};
