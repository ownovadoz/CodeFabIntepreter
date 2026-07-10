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

void Checker::check(const vector<unique_ptr<Statement>>& statements)
{
    for (const auto& statement : statements)
    {
        if (statement)
        {
            resolveStmt(statement.get());
        }
    }
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

void Checker::visitFunctionStmt(const FunctionStmt& stmt)
{
    declare(stmt.getName());
    define(stmt.getName());

    resolveFunction(stmt);
}

void Checker::resolveFunction(const FunctionStmt& stmt, bool is_initializer)
{
    ScopeGuard scope_guard(*this);
    FunctionGuard function_guard(*this, is_initializer);

    for (const Token& param : stmt.getParams())
    {
        declare(param);
        define(param);
    }

    for (const auto& body_stmt : stmt.getBody()->getStatements())
        resolveStmt(body_stmt.get());
}

void Checker::visitReturnStmt(const ReturnStmt& stmt)
{
    if (function_depth == 0)
        throw CodeFabException(stmt.getKeyword(), "함수 외부에서 return을 사용할 수 없습니다.");

    if (in_initializer)
        throw CodeFabException(stmt.getKeyword(), "생성자(init)에서는 return을 사용할 수 없습니다.");

    resolveExpr(stmt.getValue());
}

void Checker::visitClassStmt(const ClassStmt& stmt)
{
    declare(stmt.getName());
    define(stmt.getName());

    const VariableExpr* superclass = stmt.getSuperclass();
    if (superclass != nullptr)
    {
        if (superclass->getToken().getLexeme() == stmt.getName().getLexeme())
            throw CodeFabException(superclass->getToken(), "클래스는 자기 자신을 상속할 수 없습니다: '" + stmt.getName().getLexeme() + "'");

        // 상속 대상이 실제로 클래스인지(변수/함수가 아닌지)는 값의 타입을 알아야 하는
        // 런타임 검사이므로, 여기서는 참조 자체만 검사하고 최종 판단은 Executor가 내린다.
        resolveExpr(superclass);
    }

    ClassGuard class_guard(*this, superclass != nullptr);
    for (const auto& method : stmt.getMethods())
        resolveFunction(*method, method->isInitializer());
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

void Checker::visitCallExpr(const CallExpr& expr)
{
    resolveExpr(expr.getCallee());

    for (const auto& argument : expr.getArguments())
        resolveExpr(argument.get());
}

void Checker::visitGetExpr(const GetExpr& expr)
{
    // 필드/메서드 이름 자체는 변수가 아니므로 검사 대상이 아니다. 실제 존재 여부는 런타임에 확인한다.
    resolveExpr(expr.getObject());
}

void Checker::visitSetExpr(const SetExpr& expr)
{
    resolveExpr(expr.getValue());
    resolveExpr(expr.getObject());
}

void Checker::visitThisExpr(const ThisExpr& expr)
{
    if (class_stack.empty())
        throw CodeFabException(expr.getKeyword(), "클래스 외부에서 this를 사용할 수 없습니다.");
}

void Checker::visitSuperExpr(const SuperExpr& expr)
{
    if (class_stack.empty())
        throw CodeFabException(expr.getKeyword(), "클래스 외부에서 Super를 사용할 수 없습니다.");

    if (!class_stack.back().has_superclass)
        throw CodeFabException(expr.getKeyword(), "부모 클래스가 없는 클래스에서는 Super를 사용할 수 없습니다.");
}

void Checker::visitInstanceOfExpr(const InstanceOfExpr& expr)
{
    resolveExpr(expr.getObject());
}

void Checker::visitArrayExpr(const ArrayExpr& expr)
{
    resolveExpr(expr.getSize());
}

void Checker::visitIndexExpr(const IndexExpr& expr)
{
    resolveExpr(expr.getArray());
    resolveExpr(expr.getIndex());
}

void Checker::visitIndexSetExpr(const IndexSetExpr& expr)
{
    resolveExpr(expr.getArray());
    resolveExpr(expr.getIndex());
    resolveExpr(expr.getValue());
}
