#pragma once

#include "Environment.h"

#include "../AssemblerUnit/Parser/Expression.h"
#include "../AssemblerUnit/Parser/Statement.h"
#include "../AssemblerUnit/Tokenizer/Value.h"
#include "../InterfaceForCodeFabTest.h"

#include <memory>
#include <string>

using std::string;
using std::shared_ptr;

#ifdef _DEBUG
class Interpreter : public IExecutor
#else
class Interpreter
#endif
{
public:
    Interpreter();

#ifdef _DEBUG
    void interpret(Statement* stmt) override;
#else
    void interpret(Statement* stmt);
#endif
    Value evaluate(const Expression* expr);
    Value getVariableValue(const string& name) const;

private:
    void execute(Statement* stmt);
    void executeBlockStmt(BlockStmt* block);
    void executeVarDeclareStmt(VarDeclareStmt* var_decl);
    void executeExpressionStmt(ExpressionStmt* stmt);
    void executePrintStmt(PrintStmt* stmt);
    void executeIfStmt(IfStmt* if_stmt);
    void executeForStmt(ForStmt* for_stmt);

    Value evaluateLiteralExpr(const LiteralExpr* literal);

    shared_ptr<Environment> globals;
    shared_ptr<Environment> environment;

};
