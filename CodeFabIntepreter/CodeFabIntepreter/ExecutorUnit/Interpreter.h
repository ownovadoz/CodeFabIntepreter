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
class Interpreter : public IExecutor, public ExprVisitor, public StmtVisitor
#else
class Interpreter : public ExprVisitor, public StmtVisitor
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
    Value evaluateVariableExpr(const VariableExpr* variable);

    void visitExpressionStmt(const ExpressionStmt& stmt) override;
    void visitIfStmt(const IfStmt& stmt) override;
    void visitBlockStmt(const BlockStmt& stmt) override;
    void visitVarDeclareStmt(const VarDeclareStmt& stmt) override;
    void visitPrintStmt(const PrintStmt& stmt) override;
    void visitForStmt(const ForStmt& stmt) override;

    void visitLiteralExpr(const LiteralExpr& expr) override;
    void visitVariableExpr(const VariableExpr& expr) override;
    void visitAssignExpr(const AssignExpr& expr) override;
    void visitBinaryExpr(const BinaryExpr& expr) override;
    void visitUnaryExpr(const UnaryExpr& expr) override;
    void visitGroupingExpr(const GroupingExpr& expr) override;
    void visitLogicalExpr(const LogicalExpr& expr) override;

    shared_ptr<Environment> globals;
    shared_ptr<Environment> environment;

    Value evaluation_result;
    bool has_evaluation_result = false;
};
