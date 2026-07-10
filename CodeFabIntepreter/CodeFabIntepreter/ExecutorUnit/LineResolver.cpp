#include "LineResolver.h"

int LineResolver::resolve(const Expression* expr)
{
    if (expr == nullptr) return 0;

    expr->accept(*this);
    return line;
}

int LineResolver::resolve(const Statement* stmt)
{
    if (stmt == nullptr) return 0;

    stmt->accept(*this);
    return line;
}

void LineResolver::visitLiteralExpr(const LiteralExpr& expr)
{
    line = expr.getToken().getLine();
}

void LineResolver::visitVariableExpr(const VariableExpr& expr)
{
    line = expr.getToken().getLine();
}

void LineResolver::visitAssignExpr(const AssignExpr& expr)
{
    line = expr.getIdentifier().getLine();
}

void LineResolver::visitBinaryExpr(const BinaryExpr& expr)
{
    line = expr.getOperator().getLine();
}

void LineResolver::visitUnaryExpr(const UnaryExpr& expr)
{
    line = expr.getOperator().getLine();
}

void LineResolver::visitGroupingExpr(const GroupingExpr& expr)
{
    line = resolve(expr.getExpr());
}

void LineResolver::visitLogicalExpr(const LogicalExpr& expr)
{
    line = expr.getOperator().getLine();
}

void LineResolver::visitCallExpr(const CallExpr& expr)
{
    line = expr.getParen().getLine();
}

void LineResolver::visitGetExpr(const GetExpr& expr)
{
    line = expr.getName().getLine();
}

void LineResolver::visitSetExpr(const SetExpr& expr)
{
    line = expr.getName().getLine();
}

void LineResolver::visitThisExpr(const ThisExpr& expr)
{
    line = expr.getKeyword().getLine();
}

void LineResolver::visitSuperExpr(const SuperExpr& expr)
{
    line = expr.getKeyword().getLine();
}

void LineResolver::visitInstanceOfExpr(const InstanceOfExpr& expr)
{
    line = expr.getKeyword().getLine();
}

void LineResolver::visitArrayExpr(const ArrayExpr& expr)
{
    line = resolve(expr.getSize());
}

void LineResolver::visitIndexExpr(const IndexExpr& expr)
{
    line = resolve(expr.getArray());
}

void LineResolver::visitIndexSetExpr(const IndexSetExpr& expr)
{
    line = resolve(expr.getArray());
}

void LineResolver::visitExpressionStmt(const ExpressionStmt& stmt)
{
    line = resolve(stmt.getExpr());
}

void LineResolver::visitPrintStmt(const PrintStmt& stmt)
{
    line = resolve(stmt.getExpr());
}

void LineResolver::visitVarDeclareStmt(const VarDeclareStmt& stmt)
{
    line = stmt.getName().getLine();
}

void LineResolver::visitIfStmt(const IfStmt& stmt)
{
    line = resolve(stmt.getCondition());
}

void LineResolver::visitBlockStmt(const BlockStmt& stmt)
{
    for (const auto& inner : stmt.getStatements()) {
        line = resolve(inner.get());
        if (line != 0) return;
    }

    line = 0;
}

void LineResolver::visitForStmt(const ForStmt& stmt)
{
    if (stmt.getInit() != nullptr) {
        line = resolve(stmt.getInit());
        return;
    }

    if (stmt.getCondition() != nullptr) {
        line = resolve(stmt.getCondition());
        return;
    }

    if (stmt.getIncrement() != nullptr) {
        line = resolve(stmt.getIncrement());
        return;
    }

    line = resolve(stmt.getBody());
}

void LineResolver::visitFunctionStmt(const FunctionStmt& stmt)
{
    line = stmt.getName().getLine();
}

void LineResolver::visitReturnStmt(const ReturnStmt& stmt)
{
    line = stmt.getKeyword().getLine();
}

void LineResolver::visitClassStmt(const ClassStmt& stmt)
{
    line = stmt.getName().getLine();
}
