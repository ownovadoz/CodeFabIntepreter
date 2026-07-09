#pragma once

#include "Environment.h"

#include "../AssemblerUnit/Parser/Expression.h"
#include "../AssemblerUnit/Parser/Statement.h"
#include "../AssemblerUnit/Tokenizer/Value.h"
#include "../InterfaceForCodeFabTest.h"
#include "../Visitor.h"

#include <string>

using std::string;

#ifdef _DEBUG
class Interpreter : public IExecutor, public StmtVisitor, public ExprVisitor
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

    Value evaluateLiteralExpr(const LiteralExpr* literal);

    void visitExpressionStmt(ExpressionStmt& stmt) override;
    void visitIfStmt(IfStmt& stmt) override;
    void visitBlockStmt(BlockStmt& stmt) override;
    void visitVarDeclareStmt(VarDeclareStmt& stmt) override;
    void visitPrintStmt(PrintStmt& stmt) override;
    void visitForStmt(ForStmt& stmt) override;

    void visitLiteralExpr(const LiteralExpr& expr) override;
    void visitVariableExpr(const VariableExpr& expr) override;
    void visitAssignExpr(const AssignExpr& expr) override;
    void visitBinaryExpr(const BinaryExpr& expr) override;
    void visitUnaryExpr(const UnaryExpr& expr) override;
    void visitGroupingExpr(const GroupingExpr& expr) override;
    void visitLogicalExpr(const LogicalExpr& expr) override;

    Environment global_environment;
    Environment* current_environment;
    Value evaluation_result;
    bool has_evaluation_result = false;
};
