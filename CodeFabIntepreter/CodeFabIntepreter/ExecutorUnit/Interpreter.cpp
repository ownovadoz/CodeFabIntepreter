#include "Interpreter.h"

#include "../CodeFabException.h"
#include <iostream>
#include <memory>
#include <variant>
using std::cout;
using std::make_shared;

Interpreter::Interpreter()
{
    globals = make_shared<Environment>();
    environment = globals;
}

void Interpreter::interpret(Statement* stmt)
{
    execute(stmt);
}

Value Interpreter::getVariableValue(const string& name) const
{
    return environment->get(name);
}

void Interpreter::execute(Statement* stmt)
{
    if (stmt == nullptr) return;

    stmt->accept(*this);
}

void Interpreter::executeBlockStmt(BlockStmt* block)
{
    shared_ptr<Environment> previous = environment;
    environment = make_shared<Environment>(previous);

    for (const auto& stmt : block->getStatements()) {
        execute(stmt.get());
    }
    environment = previous;
}

void Interpreter::executeVarDeclareStmt(VarDeclareStmt* var_decl)
{
    Value value;
    if (const Expression* initializer = var_decl->getInitializer()) {
        value = evaluate(initializer);
    }

    environment->define(var_decl->getName().getLexeme(), value);
}

void Interpreter::executeExpressionStmt(ExpressionStmt* stmt)
{
    evaluate(stmt->getExpr());
}

void Interpreter::executePrintStmt(PrintStmt* stmt)
{
    Value value = evaluate(stmt->getExpr());
    cout << stringify(value) << "\n";
}

void Interpreter::executeIfStmt(IfStmt* if_stmt)
{
    if (isTruthy(evaluate(if_stmt->getCondition()))) {
        execute(const_cast<Statement*>(if_stmt->getThenBranch()));
    }
    else {
        execute(const_cast<Statement*>(if_stmt->getElseBranch()));
    }
}

void Interpreter::executeForStmt(ForStmt* for_stmt)
{
    shared_ptr<Environment> previous = environment;
    environment = make_shared<Environment>(previous);

    if (for_stmt->getInit() != nullptr) {
        executeVarDeclareStmt(const_cast<VarDeclareStmt*>(for_stmt->getInit()));
    }

    while (for_stmt->getCondition() == nullptr || isTruthy(evaluate(for_stmt->getCondition()))) {
        execute(const_cast<Statement*>(for_stmt->getBody()));

        if (for_stmt->getIncrement() != nullptr) {
            evaluate(for_stmt->getIncrement());
        }
    }

    environment = previous;
}

Value Interpreter::evaluate(const Expression* expr)
{
    has_evaluation_result = false;
    expr->accept(*this);

    if (!has_evaluation_result) throw CodeFabException(0, "지원하지 않는 표현식입니다.");

    return evaluation_result;
}

Value Interpreter::evaluateLiteralExpr(const LiteralExpr* literal)
{
    return literal->getToken().getLiteral();
}

Value Interpreter::evaluateVariableExpr(const VariableExpr* variable)
{
    return environment->get(variable->getToken().getLexeme());
}

void Interpreter::visitExpressionStmt(const ExpressionStmt& stmt)
{
    executeExpressionStmt(const_cast<ExpressionStmt*>(&stmt));
}

void Interpreter::visitIfStmt(const IfStmt& stmt)
{
    executeIfStmt(const_cast<IfStmt*>(&stmt));
}

void Interpreter::visitBlockStmt(const BlockStmt& stmt)
{
    executeBlockStmt(const_cast<BlockStmt*>(&stmt));
}

void Interpreter::visitVarDeclareStmt(const VarDeclareStmt& stmt)
{
    executeVarDeclareStmt(const_cast<VarDeclareStmt*>(&stmt));
}

void Interpreter::visitPrintStmt(const PrintStmt& stmt)
{
    executePrintStmt(const_cast<PrintStmt*>(&stmt));
}

void Interpreter::visitForStmt(const ForStmt& stmt)
{
    executeForStmt(const_cast<ForStmt*>(&stmt));
}

void Interpreter::visitLiteralExpr(const LiteralExpr& expr)
{
    evaluation_result = evaluateLiteralExpr(&expr);
    has_evaluation_result = true;
}

void Interpreter::visitVariableExpr(const VariableExpr& expr)
{
    evaluation_result = evaluateVariableExpr(&expr);
    has_evaluation_result = true;
}

void Interpreter::visitAssignExpr(const AssignExpr& expr)
{
    evaluation_result = evaluateAssignExpr(expr);
    has_evaluation_result = true;
}

Value Interpreter::evaluateAssignExpr(const AssignExpr& expr)
{
    Value value = evaluate(expr.getValue());
    environment->assign(expr.getIdentifier().getLexeme(), value);
    return value;
}
void Interpreter::visitBinaryExpr(const BinaryExpr& expr)
{
    evaluation_result = evaluateBinaryExpr(expr);
    has_evaluation_result = true;
}

Value Interpreter::evaluateBinaryExpr(const BinaryExpr& expr)
{
    Value left = evaluate(expr.getLeft());
    Value right = evaluate(expr.getRight());
    const Token& op = expr.getOperator();

    switch (op.getType()) {
    case TokenType::PLUS:
        if (isNumber(left) && isNumber(right)) return std::get<double>(left) + std::get<double>(right);
        if (isString(left) && isString(right)) return std::get<string>(left) + std::get<string>(right);
        throw CodeFabException(op, "피연산자는 둘 다 숫자이거나 둘 다 문자열이어야 합니다.");

    case TokenType::MINUS:
        ensureNumberOperands(op, left, right);
        return std::get<double>(left) - std::get<double>(right);

    case TokenType::STAR:
        ensureNumberOperands(op, left, right);
        return std::get<double>(left) * std::get<double>(right);

    case TokenType::SLASH:
        ensureNumberOperands(op, left, right);
        if (std::get<double>(right) == 0.0) throw CodeFabException(op, "0으로 나눌 수 없습니다.");
        return std::get<double>(left) / std::get<double>(right);

    case TokenType::GREATER:
        ensureNumberOperands(op, left, right);
        return std::get<double>(left) > std::get<double>(right);

    case TokenType::LESS:
        ensureNumberOperands(op, left, right);
        return std::get<double>(left) < std::get<double>(right);

    case TokenType::GREATER_EQUAL:
        ensureNumberOperands(op, left, right);
        return std::get<double>(left) >= std::get<double>(right);

    case TokenType::LESS_EQUAL:
        ensureNumberOperands(op, left, right);
        return std::get<double>(left) <= std::get<double>(right);

    case TokenType::EQUAL_EQUAL:
        return left == right;

    case TokenType::BANG_EQUAL:
        return left != right;

    default:
        throw CodeFabException(op, "지원하지 않는 이항 연산자입니다.");
    }
}

void Interpreter::ensureNumberOperands(const Token& op, const Value& left, const Value& right) const
{
    if (isNumber(left) && isNumber(right)) return;

    throw CodeFabException(op, "피연산자는 반드시 숫자여야 합니다.");
}
void Interpreter::visitUnaryExpr(const UnaryExpr&) {}
void Interpreter::visitGroupingExpr(const GroupingExpr&) {}
void Interpreter::visitLogicalExpr(const LogicalExpr&) {}
