#pragma once

#include "Environment.h"

#include "../AssemblerUnit/Parser/Expression.h"
#include "../AssemblerUnit/Parser/Statement.h"
#include "../AssemblerUnit/Tokenizer/Value.h"

#include <string>

using std::string;

class Interpreter {
public:
    Interpreter();

    void interpret(Statement* stmt);
    Value evaluate(const Expression* expr);
    Value getVariableValue(const string& name) const;

private:
    void execute(Statement* stmt);
    void executeBlockStmt(BlockStmt* block);
    void executeVarDeclareStmt(VarDeclareStmt* var_decl);

    Value evaluateLiteralExpr(const LiteralExpr* literal);

    Environment global_environment;
    Environment* current_environment;
};
