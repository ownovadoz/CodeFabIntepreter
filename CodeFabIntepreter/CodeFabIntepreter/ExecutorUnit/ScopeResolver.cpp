#include "ScopeResolver.h"

void ScopeResolver::resolve(const vector<unique_ptr<Statement>>& statements)
{
    for (const auto& statement : statements)
        resolveStmt(statement.get());
}

void ScopeResolver::beginScope()
{
    scopes.emplace_back();
}

void ScopeResolver::endScope()
{
    scopes.pop_back();
}

void ScopeResolver::declareName(const string& name)
{
    if (scopes.empty()) return;

    scopes.back()[name] = true;
}

void ScopeResolver::declare(const Token& name)
{
    declareName(name.getLexeme());
}

void ScopeResolver::resolveLocal(const Expression& expr, const string& name)
{
    for (int i = static_cast<int>(scopes.size()) - 1; i >= 0; i--) {
        if (scopes[i].find(name) == scopes[i].end()) continue;

        locals[&expr] = static_cast<int>(scopes.size()) - 1 - i;
        return;
    }
}

void ScopeResolver::resolveStmt(const Statement* stmt)
{
    if (stmt == nullptr) return;

    stmt->accept(*this);
}

void ScopeResolver::resolveExpr(const Expression* expr)
{
    if (expr == nullptr) return;

    expr->accept(*this);
}

void ScopeResolver::resolveFunctionBody(const FunctionStmt& stmt)
{
    // CodeFabFunction::call()이 파라미터와 몸통을 같은 Environment 하나에 담으므로,
    // 몸통 BlockStmt를 resolveStmt로 넘겨 별도 스코프를 여는 대신 문장들을 직접 순회한다.
    beginScope();

    for (const Token& param : stmt.getParams())
        declare(param);

    for (const auto& body_stmt : stmt.getBody()->getStatements())
        resolveStmt(body_stmt.get());

    endScope();
}

void ScopeResolver::visitExpressionStmt(const ExpressionStmt& stmt)
{
    resolveExpr(stmt.getExpr());
}

void ScopeResolver::visitIfStmt(const IfStmt& stmt)
{
    resolveExpr(stmt.getCondition());

    // Interpreter::executeIfStmt는 분기 자체를 위한 Environment를 만들지 않는다.
    // 분기가 BlockStmt라면 그 안에서 visitBlockStmt가 스코프를 열 뿐이다.
    resolveStmt(stmt.getThenBranch());
    resolveStmt(stmt.getElseBranch());
}

void ScopeResolver::visitBlockStmt(const BlockStmt& stmt)
{
    beginScope();

    for (const auto& inner : stmt.getStatements())
        resolveStmt(inner.get());

    endScope();
}

void ScopeResolver::visitVarDeclareStmt(const VarDeclareStmt& stmt)
{
    // Interpreter도 초기화식을 먼저 평가한 뒤에 변수를 정의하므로, 초기화식
    // 안에서 같은 이름을 참조해도 아직 선언되지 않은 바깥쪽 변수를 가리킨다.
    resolveExpr(stmt.getInitializer());
    declare(stmt.getName());
}

void ScopeResolver::visitPrintStmt(const PrintStmt& stmt)
{
    resolveExpr(stmt.getExpr());
}

void ScopeResolver::visitForStmt(const ForStmt& stmt)
{
    // executeForStmt는 반복문 전체를 위한 Environment를 한 번만 만들고 그 안에서
    // init을 실행한 뒤, 매 반복마다 body를 실행한다(body가 다시 Block이면 그 안에서
    // 스코프가 하나 더 열린다). 정적 분석이므로 body는 한 번만 훑으면 충분하다.
    beginScope();

    resolveStmt(stmt.getInit());
    resolveExpr(stmt.getCondition());
    resolveExpr(stmt.getIncrement());
    resolveStmt(stmt.getBody());

    endScope();
}

void ScopeResolver::visitFunctionStmt(const FunctionStmt& stmt)
{
    declare(stmt.getName());
    resolveFunctionBody(stmt);
}

void ScopeResolver::visitReturnStmt(const ReturnStmt& stmt)
{
    resolveExpr(stmt.getValue());
}

void ScopeResolver::visitClassStmt(const ClassStmt& stmt)
{
    declare(stmt.getName());

    if (stmt.getSuperclass() != nullptr)
        resolveExpr(stmt.getSuperclass());

    // executeClassStmt는 상속이 있을 때만 "super"를 담는 method_environment를 만든다.
    bool has_superclass = stmt.getSuperclass() != nullptr;
    if (has_superclass) {
        beginScope();
        declareName("super");
    }

    // CodeFabFunction::bind()는 상속 여부와 무관하게 항상 "this" 하나짜리
    // Environment를 새로 만들어 메서드 closure로 감싼다.
    beginScope();
    declareName("this");

    for (const auto& method : stmt.getMethods())
        resolveFunctionBody(*method);

    endScope();

    if (has_superclass) endScope();
}

void ScopeResolver::visitLiteralExpr(const LiteralExpr&)
{
}

void ScopeResolver::visitVariableExpr(const VariableExpr& expr)
{
    resolveLocal(expr, expr.getToken().getLexeme());
}

void ScopeResolver::visitAssignExpr(const AssignExpr& expr)
{
    resolveExpr(expr.getValue());
    resolveLocal(expr, expr.getIdentifier().getLexeme());
}

void ScopeResolver::visitBinaryExpr(const BinaryExpr& expr)
{
    resolveExpr(expr.getLeft());
    resolveExpr(expr.getRight());
}

void ScopeResolver::visitUnaryExpr(const UnaryExpr& expr)
{
    resolveExpr(expr.getExpr());
}

void ScopeResolver::visitGroupingExpr(const GroupingExpr& expr)
{
    resolveExpr(expr.getExpr());
}

void ScopeResolver::visitLogicalExpr(const LogicalExpr& expr)
{
    resolveExpr(expr.getLeft());
    resolveExpr(expr.getRight());
}

void ScopeResolver::visitCallExpr(const CallExpr& expr)
{
    resolveExpr(expr.getCallee());

    for (const auto& argument : expr.getArguments())
        resolveExpr(argument.get());
}

void ScopeResolver::visitGetExpr(const GetExpr& expr)
{
    // 필드 이름은 변수가 아니라 인스턴스 안에서 동적으로 찾는 대상이므로 거리 계산이 필요 없다.
    resolveExpr(expr.getObject());
}

void ScopeResolver::visitSetExpr(const SetExpr& expr)
{
    resolveExpr(expr.getValue());
    resolveExpr(expr.getObject());
}

void ScopeResolver::visitThisExpr(const ThisExpr& expr)
{
    resolveLocal(expr, "this");
}

void ScopeResolver::visitSuperExpr(const SuperExpr& expr)
{
    resolveLocal(expr, "super");
}

void ScopeResolver::visitInstanceOfExpr(const InstanceOfExpr& expr)
{
    resolveExpr(expr.getObject());
}

void ScopeResolver::visitArrayExpr(const ArrayExpr& expr)
{
    resolveExpr(expr.getSize());
}

void ScopeResolver::visitIndexExpr(const IndexExpr& expr)
{
    resolveExpr(expr.getArray());
    resolveExpr(expr.getIndex());
}

void ScopeResolver::visitIndexSetExpr(const IndexSetExpr& expr)
{
    resolveExpr(expr.getArray());
    resolveExpr(expr.getIndex());
    resolveExpr(expr.getValue());
}
