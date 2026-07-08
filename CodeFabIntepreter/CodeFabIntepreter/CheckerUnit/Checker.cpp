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

    stmt->accept(*this);
}

void Checker::visitBlockStmt(BlockStmt& stmt) { checkBlockStmt(&stmt); }
void Checker::visitVarDeclareStmt(VarDeclareStmt& stmt) { checkVarDeclareStmt(&stmt); }
void Checker::visitIfStmt(IfStmt& stmt) { checkIfStmt(&stmt); }
void Checker::visitExpressionStmt(ExpressionStmt&) {}
void Checker::visitPrintStmt(PrintStmt&) {}
void Checker::visitForStmt(ForStmt&) {}
void Checker::visitFunctionDeclStmt(FunctionDeclStmt&) {}
void Checker::visitReturnStmt(ReturnStmt&) {}
void Checker::visitClassDeclStmt(ClassDeclStmt&) {}
void Checker::visitImportStmt(ImportStmt&) {}

void Checker::checkBlockStmt(BlockStmt* block)
{
    ScopeGuard guard(*this);

    for (const auto& stmt : block->getStatements())
        checkStatement(stmt.get());
}

void Checker::checkVarDeclareStmt(VarDeclareStmt* var_decl)
{
    declareVariable(var_decl->getName(), {});
}

void Checker::checkIfStmt(IfStmt* if_stmt)
{
    checkStatement(const_cast<Statement*>(if_stmt->getThenBranch()));
    checkStatement(const_cast<Statement*>(if_stmt->getElseBranch()));
}
