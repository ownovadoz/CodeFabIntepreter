#include "Interpreter.h"

#include "../RuntimeError.h"

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
    // TODO: dispatch to each Statement kind once tests describe the expected behavior.
}

void Interpreter::executeBlockStmt(BlockStmt* block)
{
    // TODO: run a block in its own child Environment.
}

void Interpreter::executeVarDeclareStmt(VarDeclareStmt* var_decl)
{
    // TODO: define the declared variable in the current Environment.
}

Value Interpreter::evaluate(const Expression* expr)
{
    throw RuntimeError("", "지원하지 않는 표현식입니다.");
}

Value Interpreter::evaluateLiteralExpr(const LiteralExpr* literal)
{
    return literal->getToken().getLiteral();
}
