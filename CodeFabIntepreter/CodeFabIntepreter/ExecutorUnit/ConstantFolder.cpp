#include "ConstantFolder.h"

#include "Interpreter.h"

void ConstantFolder::fold(const vector<unique_ptr<Statement>>& statements)
{
    for (const auto& statement : statements)
        resolveStmt(statement.get());
}

void ConstantFolder::resolveStmt(const Statement* stmt)
{
    if (stmt == nullptr) return;

    stmt->accept(*this);
}

void ConstantFolder::resolveExpr(const Expression* expr)
{
    if (expr == nullptr) return;

    expr->accept(*this);
}

bool ConstantFolder::isConstant(const Expression* expr) const
{
    if (expr == nullptr) return false;
    if (dynamic_cast<const LiteralExpr*>(expr) != nullptr) return true;

    return folded.find(expr) != folded.end();
}

void ConstantFolder::tryFold(const Expression& expr)
{
    // 자식이 전부 리터럴이므로 부작용(변수 접근 등)이 없다는 것이 보장된다. 그래서
    // 연산 로직을 여기서 새로 베끼는 대신 Interpreter::evaluate를 그대로 재사용해
    // 실제 실행 결과와 절대 어긋나지 않게 한다. 0으로 나누기처럼 실행 중 오류가
    // 나는 연산은 여기서 삼키고 접지 않아, 실제 실행 시점에 원래와 동일한 예외와
    // 줄 번호로 보고된다.
    try {
        folded[&expr] = interpreter.evaluate(&expr);
    }
    catch (...) {
    }
}

void ConstantFolder::visitLiteralExpr(const LiteralExpr&)
{
}

void ConstantFolder::visitVariableExpr(const VariableExpr&)
{
}

void ConstantFolder::visitAssignExpr(const AssignExpr& expr)
{
    resolveExpr(expr.getValue());
}

void ConstantFolder::visitBinaryExpr(const BinaryExpr& expr)
{
    resolveExpr(expr.getLeft());
    resolveExpr(expr.getRight());

    if (isConstant(expr.getLeft()) && isConstant(expr.getRight())) tryFold(expr);
}

void ConstantFolder::visitUnaryExpr(const UnaryExpr& expr)
{
    resolveExpr(expr.getExpr());

    if (isConstant(expr.getExpr())) tryFold(expr);
}

void ConstantFolder::visitGroupingExpr(const GroupingExpr& expr)
{
    resolveExpr(expr.getExpr());

    if (isConstant(expr.getExpr())) tryFold(expr);
}

void ConstantFolder::visitLogicalExpr(const LogicalExpr& expr)
{
    resolveExpr(expr.getLeft());
    resolveExpr(expr.getRight());

    if (isConstant(expr.getLeft()) && isConstant(expr.getRight())) tryFold(expr);
}

void ConstantFolder::visitCallExpr(const CallExpr& expr)
{
    resolveExpr(expr.getCallee());

    for (const auto& argument : expr.getArguments())
        resolveExpr(argument.get());
}

void ConstantFolder::visitGetExpr(const GetExpr& expr)
{
    resolveExpr(expr.getObject());
}

void ConstantFolder::visitSetExpr(const SetExpr& expr)
{
    resolveExpr(expr.getObject());
    resolveExpr(expr.getValue());
}

void ConstantFolder::visitThisExpr(const ThisExpr&)
{
}

void ConstantFolder::visitSuperExpr(const SuperExpr&)
{
}

void ConstantFolder::visitInstanceOfExpr(const InstanceOfExpr& expr)
{
    resolveExpr(expr.getObject());
}

void ConstantFolder::visitArrayExpr(const ArrayExpr& expr)
{
    resolveExpr(expr.getSize());
}

void ConstantFolder::visitIndexExpr(const IndexExpr& expr)
{
    resolveExpr(expr.getArray());
    resolveExpr(expr.getIndex());
}

void ConstantFolder::visitIndexSetExpr(const IndexSetExpr& expr)
{
    resolveExpr(expr.getArray());
    resolveExpr(expr.getIndex());
    resolveExpr(expr.getValue());
}

void ConstantFolder::visitExpressionStmt(const ExpressionStmt& stmt)
{
    resolveExpr(stmt.getExpr());
}

void ConstantFolder::visitIfStmt(const IfStmt& stmt)
{
    resolveExpr(stmt.getCondition());
    resolveStmt(stmt.getThenBranch());
    resolveStmt(stmt.getElseBranch());
}

void ConstantFolder::visitBlockStmt(const BlockStmt& stmt)
{
    for (const auto& inner : stmt.getStatements())
        resolveStmt(inner.get());
}

void ConstantFolder::visitVarDeclareStmt(const VarDeclareStmt& stmt)
{
    resolveExpr(stmt.getInitializer());
}

void ConstantFolder::visitPrintStmt(const PrintStmt& stmt)
{
    resolveExpr(stmt.getExpr());
}

void ConstantFolder::visitForStmt(const ForStmt& stmt)
{
    resolveStmt(stmt.getInit());
    resolveExpr(stmt.getCondition());
    resolveExpr(stmt.getIncrement());
    resolveStmt(stmt.getBody());
}

void ConstantFolder::visitFunctionStmt(const FunctionStmt& stmt)
{
    for (const auto& body_stmt : stmt.getBody()->getStatements())
        resolveStmt(body_stmt.get());
}

void ConstantFolder::visitReturnStmt(const ReturnStmt& stmt)
{
    resolveExpr(stmt.getValue());
}

void ConstantFolder::visitClassStmt(const ClassStmt& stmt)
{
    resolveExpr(stmt.getSuperclass());

    for (const auto& method : stmt.getMethods())
        for (const auto& body_stmt : method->getBody()->getStatements())
            resolveStmt(body_stmt.get());
}

void ConstantFolder::visitImportStmt(const ImportStmt&)
{
    // import 문 자체에는 접을 수 있는 연산식이 없다. import된 파일의 문장은
    // Interpreter::executeImportStmt가 별도로 interpret()하면서 그 안에서 다시
    // 상수 폴딩된다.
}
