#include "Interpreter.h"

#include "../RuntimeError.h"

namespace {

// 블록 실행 도중 예외가 발생해도 current_environment가 항상 이전 스코프로 복원되도록 보장한다.
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

    if (BlockStmt* block = dynamic_cast<BlockStmt*>(stmt)) {
        executeBlockStmt(block);
        return;
    }

    if (VarDeclareStmt* var_decl = dynamic_cast<VarDeclareStmt*>(stmt)) {
        executeVarDeclareStmt(var_decl);
        return;
    }
}

void Interpreter::executeBlockStmt(BlockStmt* block)
{
    Environment block_environment(current_environment);
    EnvironmentGuard guard(current_environment, &block_environment);

    for (Statement* stmt : block->getStatements()) {
        execute(stmt);
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
    if (const LiteralExpr* literal = dynamic_cast<const LiteralExpr*>(expr)) {
        return evaluateLiteralExpr(literal);
    }

    throw RuntimeError("", "지원하지 않는 표현식입니다.");
}

Value Interpreter::evaluateLiteralExpr(const LiteralExpr* literal)
{
    return literal->getToken().getLiteral();
}
