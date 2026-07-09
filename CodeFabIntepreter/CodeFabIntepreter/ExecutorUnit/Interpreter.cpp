#include "Interpreter.h"

#include "../CodeFabException.h"
#include <iostream>
#include <memory>
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

    if (BlockStmt* block = dynamic_cast<BlockStmt*>(stmt)) {
        executeBlockStmt(block);
        return;
    }

    if (VarDeclareStmt* var_decl = dynamic_cast<VarDeclareStmt*>(stmt)) {
        executeVarDeclareStmt(var_decl);
        return;
    }

    if (ExpressionStmt* expr_stmt = dynamic_cast<ExpressionStmt*>(stmt)) {
        executeExpressionStmt(expr_stmt);
        return;
    }

    if (PrintStmt* print_stmt = dynamic_cast<PrintStmt*>(stmt)) {
        executePrintStmt(print_stmt);
        return;
    }

    if (IfStmt* if_stmt = dynamic_cast<IfStmt*>(stmt)) {
        executeIfStmt(if_stmt);
        return;
    }

    if (ForStmt* for_stmt = dynamic_cast<ForStmt*>(stmt)) {
        executeForStmt(for_stmt);
        return;
    }
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
    if (const LiteralExpr* literal = dynamic_cast<const LiteralExpr*>(expr)) {
        return evaluateLiteralExpr(literal);
    }

    if (const VariableExpr* variable = dynamic_cast<const VariableExpr*>(expr)) {
        return evaluateVariableExpr(variable);
    }

    throw CodeFabException(0, "지원하지 않는 표현식입니다.");
}

Value Interpreter::evaluateLiteralExpr(const LiteralExpr* literal)
{
    return literal->getToken().getLiteral();
}

Value Interpreter::evaluateVariableExpr(const VariableExpr* variable)
{
    return environment->get(variable->getToken().getLexeme());
}
