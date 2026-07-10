#include "Resolver.h"

void Resolver::resolve(const vector<unique_ptr<Statement>>& statements)
{
    for (const auto& statement : statements)
        resolveStmt(statement.get());
}

const int* Resolver::getDistance(const Expression* expr) const
{
    auto found = locals.find(expr);
    return found != locals.end() ? &found->second : nullptr;
}

void Resolver::beginScope()
{
    scopes.emplace_back();
}

void Resolver::endScope()
{
    if (scopes.empty()) return;

    scopes.pop_back();
}

void Resolver::declare(const Token& name)
{
    if (scopes.empty()) return;

    scopes.back()[name.getLexeme()] = false;
}

void Resolver::define(const Token& name)
{
    if (scopes.empty()) return;

    scopes.back()[name.getLexeme()] = true;
}

void Resolver::resolveLocal(const Expression& expr, const string& name)
{
    for (int i = static_cast<int>(scopes.size()) - 1; i >= 0; i--) {
        if (scopes[i].find(name) != scopes[i].end()) {
            locals[&expr] = static_cast<int>(scopes.size()) - 1 - i;
            return;
        }
    }

    // 어느 지역 스코프에서도 찾지 못했다면 전역 변수다. 거리를 남겨두지 않고,
    // Environment::get/assign의 기존 이름 기반 탐색이 처리하도록 그대로 둔다.
}

void Resolver::resolveStmt(const Statement* stmt)
{
    if (stmt == nullptr) return;

    stmt->accept(*this);
}

void Resolver::resolveExpr(const Expression* expr)
{
    if (expr == nullptr) return;

    expr->accept(*this);
}

void Resolver::resolveFunction(const FunctionStmt& stmt)
{
    // CodeFabFunction::call이 파라미터와 몸통을 하나의 Environment에서 함께
    // 실행하므로(몸통을 위한 별도 블록 스코프를 추가로 열지 않는다), 여기서도
    // 파라미터 선언과 몸통 안 문장 해석을 같은 스코프 하나에서 처리한다.
    ScopeGuard scope_guard(*this);

    for (const Token& param : stmt.getParams()) {
        declare(param);
        define(param);
    }

    for (const auto& body_stmt : stmt.getBody()->getStatements())
        resolveStmt(body_stmt.get());
}

void Resolver::visitLiteralExpr(const LiteralExpr&)
{
}

void Resolver::visitVariableExpr(const VariableExpr& expr)
{
    resolveLocal(expr, expr.getToken().getLexeme());
}

void Resolver::visitAssignExpr(const AssignExpr& expr)
{
    resolveExpr(expr.getValue());
    resolveLocal(expr, expr.getIdentifier().getLexeme());
}

void Resolver::visitBinaryExpr(const BinaryExpr& expr)
{
    resolveExpr(expr.getLeft());
    resolveExpr(expr.getRight());
}

void Resolver::visitUnaryExpr(const UnaryExpr& expr)
{
    resolveExpr(expr.getExpr());
}

void Resolver::visitGroupingExpr(const GroupingExpr& expr)
{
    resolveExpr(expr.getExpr());
}

void Resolver::visitLogicalExpr(const LogicalExpr& expr)
{
    resolveExpr(expr.getLeft());
    resolveExpr(expr.getRight());
}

void Resolver::visitCallExpr(const CallExpr& expr)
{
    resolveExpr(expr.getCallee());

    for (const auto& argument : expr.getArguments())
        resolveExpr(argument.get());
}

void Resolver::visitGetExpr(const GetExpr& expr)
{
    // 필드/메서드 이름 자체는 변수가 아니므로 해석 대상이 아니다.
    resolveExpr(expr.getObject());
}

void Resolver::visitSetExpr(const SetExpr& expr)
{
    resolveExpr(expr.getValue());
    resolveExpr(expr.getObject());
}

void Resolver::visitThisExpr(const ThisExpr& expr)
{
    resolveLocal(expr, "this");
}

void Resolver::visitSuperExpr(const SuperExpr& expr)
{
    resolveLocal(expr, "super");
}

void Resolver::visitInstanceOfExpr(const InstanceOfExpr& expr)
{
    resolveExpr(expr.getObject());
    resolveLocal(expr, expr.getClassName().getLexeme());
}

void Resolver::visitArrayExpr(const ArrayExpr& expr)
{
    resolveExpr(expr.getSize());
}

void Resolver::visitIndexExpr(const IndexExpr& expr)
{
    resolveExpr(expr.getArray());
    resolveExpr(expr.getIndex());
}

void Resolver::visitIndexSetExpr(const IndexSetExpr& expr)
{
    resolveExpr(expr.getArray());
    resolveExpr(expr.getIndex());
    resolveExpr(expr.getValue());
}

void Resolver::visitExpressionStmt(const ExpressionStmt& stmt)
{
    resolveExpr(stmt.getExpr());
}

void Resolver::visitIfStmt(const IfStmt& stmt)
{
    // Interpreter::executeIfStmt는 분기 문장이 BlockStmt일 때만 새 Environment를
    // 만드므로(visitBlockStmt 참고), 여기서도 분기 자체에는 별도 스코프를 열지
    // 않고 실제 실행 시점의 중첩 구조를 그대로 따라간다.
    resolveExpr(stmt.getCondition());
    resolveStmt(stmt.getThenBranch());
    resolveStmt(stmt.getElseBranch());
}

void Resolver::visitBlockStmt(const BlockStmt& stmt)
{
    ScopeGuard guard(*this);

    for (const auto& child : stmt.getStatements())
        resolveStmt(child.get());
}

void Resolver::visitVarDeclareStmt(const VarDeclareStmt& stmt)
{
    declare(stmt.getName());
    resolveExpr(stmt.getInitializer());
    define(stmt.getName());
}

void Resolver::visitPrintStmt(const PrintStmt& stmt)
{
    resolveExpr(stmt.getExpr());
}

void Resolver::visitForStmt(const ForStmt& stmt)
{
    // for 문의 초기화 변수는 Interpreter::executeForStmt가 만드는 전용
    // Environment 하나에 속하며, 몸통은(그 자체가 BlockStmt가 아닌 한) 별도
    // 스코프 없이 그 안에서 바로 실행된다.
    ScopeGuard guard(*this);

    resolveStmt(stmt.getInit());
    resolveExpr(stmt.getCondition());
    resolveExpr(stmt.getIncrement());
    resolveStmt(stmt.getBody());
}

void Resolver::visitFunctionStmt(const FunctionStmt& stmt)
{
    declare(stmt.getName());
    define(stmt.getName());

    resolveFunction(stmt);
}

void Resolver::visitReturnStmt(const ReturnStmt& stmt)
{
    resolveExpr(stmt.getValue());
}

void Resolver::visitClassStmt(const ClassStmt& stmt)
{
    declare(stmt.getName());
    define(stmt.getName());

    const VariableExpr* superclass = stmt.getSuperclass();
    if (superclass != nullptr)
        resolveExpr(superclass);

    // executeClassStmt가 상속이 있을 때만 "super"를 바인딩하는 Environment를
    // 하나 더 끼워 넣고, bind()가 메서드마다 "this"를 바인딩하는 Environment를
    // 그 안쪽에 또 하나 끼워 넣는 것과 동일한 순서로 스코프를 연다.
    if (superclass != nullptr) {
        beginScope();
        scopes.back()["super"] = true;
    }

    beginScope();
    scopes.back()["this"] = true;

    for (const auto& method : stmt.getMethods())
        resolveFunction(*method);

    endScope();

    if (superclass != nullptr)
        endScope();
}
