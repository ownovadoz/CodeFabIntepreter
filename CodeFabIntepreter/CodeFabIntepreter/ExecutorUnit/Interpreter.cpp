#include "Interpreter.h"

#include "../CodeFabException.h"

namespace {
class EnvironmentGuard {
public:
    EnvironmentGuard(Environment*& current, Environment* block_environment)
        : current(current), previous(current)
    {
        current = block_environment;
    }

    ~EnvironmentGuard()
    {
        current = previous;
    }

private:
    Environment*& current;
    Environment* previous;
};

}

Interpreter::Interpreter()
    : global_environment(nullptr), current_environment(&global_environment)
{}

void Interpreter::interpret(Statement* stmt)
{
    execute(stmt);
}

Value Interpreter::getVariableValue(const string& name) const
{
    return current_environment->get(name);
}

void Interpreter::execute(Statement* stmt)
{
    if (stmt == nullptr) return;

    stmt->accept(*this);
}

void Interpreter::visitBlockStmt(BlockStmt& stmt) { executeBlockStmt(&stmt); }
void Interpreter::visitVarDeclareStmt(VarDeclareStmt& stmt) { executeVarDeclareStmt(&stmt); }
void Interpreter::visitExpressionStmt(ExpressionStmt&) {}
void Interpreter::visitIfStmt(IfStmt&) {}
void Interpreter::visitPrintStmt(PrintStmt&) {}
void Interpreter::visitForStmt(ForStmt&) {}
void Interpreter::visitFunctionDeclStmt(FunctionDeclStmt&) {}
void Interpreter::visitReturnStmt(ReturnStmt&) {}
void Interpreter::visitClassDeclStmt(ClassDeclStmt&) {}
void Interpreter::visitImportStmt(ImportStmt&) {}

void Interpreter::executeBlockStmt(BlockStmt* block)
{
    Environment block_environment(current_environment);
    EnvironmentGuard guard(current_environment, &block_environment);

    for (const auto& stmt : block->getStatements()) {
        execute(stmt.get());
    }
}

void Interpreter::executeVarDeclareStmt(VarDeclareStmt* var_decl)
{
    Value value;
    if (const Expression* initializer = var_decl->getInitializer()) {
        value = evaluate(initializer);
    }

    current_environment->define(var_decl->getName().getLexeme(), value);
}

Value Interpreter::evaluate(const Expression* expr)
{
    has_evaluation_result = false;
    expr->accept(*this);

    if (!has_evaluation_result) throw CodeFabException(0, "지원하지 않는 표현식입니다.");

    return evaluation_result;
}

void Interpreter::visitLiteralExpr(const LiteralExpr& expr)
{
    evaluation_result = evaluateLiteralExpr(&expr);
    has_evaluation_result = true;
}

void Interpreter::visitVariableExpr(const VariableExpr&) {}
void Interpreter::visitAssignExpr(const AssignExpr&) {}
void Interpreter::visitBinaryExpr(const BinaryExpr&) {}
void Interpreter::visitUnaryExpr(const UnaryExpr&) {}
void Interpreter::visitGroupingExpr(const GroupingExpr&) {}
void Interpreter::visitLogicalExpr(const LogicalExpr&) {}
void Interpreter::visitArrayExpr(const ArrayExpr&) {}
void Interpreter::visitIndexExpr(const IndexExpr&) {}
void Interpreter::visitIndexSetExpr(const IndexSetExpr&) {}
void Interpreter::visitCallExpr(const CallExpr&) {}
void Interpreter::visitGetExpr(const GetExpr&) {}
void Interpreter::visitSetExpr(const SetExpr&) {}
void Interpreter::visitThisExpr(const ThisExpr&) {}
void Interpreter::visitSuperExpr(const SuperExpr&) {}
void Interpreter::visitInstanceOfExpr(const InstanceOfExpr&) {}

Value Interpreter::evaluateLiteralExpr(const LiteralExpr* literal)
{
    return literal->getToken().getLiteral();
}
