#pragma once

#include "../AssemblerUnit/Parser/Statement.h"
#include "../Visitor.h"
#include "../InterfaceForCodeFabTest.h"

#include <set>
#include <string>
#include <vector>

using std::set;
using std::string;
using std::vector;

#ifdef _DEBUG
class Checker : public IChecker, StmtVisitor
#else
class Checker
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

private:
    void enterScope();
    void exitScope();

    void checkStatement(Statement* stmt);
    void checkBlockStmt(BlockStmt* block);
    void checkVarDeclareStmt(VarDeclareStmt* var_decl);
    void checkIfStmt(IfStmt* if_stmt);

    bool isDeclaredInCurrentScope(const string& name) const;

    void visitExpressionStmt(ExpressionStmt& stmt) override;
    void visitIfStmt(IfStmt& stmt) override;
    void visitBlockStmt(BlockStmt& stmt) override;
    void visitVarDeclareStmt(VarDeclareStmt& stmt) override;
    void visitPrintStmt(PrintStmt& stmt) override;
    void visitForStmt(ForStmt& stmt) override;
    void visitFunctionDeclStmt(FunctionDeclStmt& stmt) override;
    void visitReturnStmt(ReturnStmt& stmt) override;
    void visitClassDeclStmt(ClassDeclStmt& stmt) override;
    void visitImportStmt(ImportStmt& stmt) override;

    vector<set<string>> scope_stack;
};
