#pragma once

#include "Environment.h"

#include "../AssemblerUnit/Parser/Expression.h"
#include "../AssemblerUnit/Parser/Statement.h"
#include "../AssemblerUnit/Tokenizer/Value.h"
#include "../InterfaceForCodeFabTest.h"
#include "../Visitor.h"

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
    Value evaluateAssignExpr(const AssignExpr& expr);
    Value evaluateBinaryExpr(const BinaryExpr& expr);
    void ensureNumberOperands(const Token& op, const Value& left, const Value& right) const;
    Value evaluateUnaryExpr(const UnaryExpr& expr);
    Value evaluateLogicalExpr(const LogicalExpr& expr);
    int resolveLine(const Expression* expr) const;

    void visitExpressionStmt(const ExpressionStmt& stmt) override;
    void visitIfStmt(const IfStmt& stmt) override;
    void visitBlockStmt(const BlockStmt& stmt) override;
    void visitVarDeclareStmt(const VarDeclareStmt& stmt) override;
    void visitPrintStmt(const PrintStmt& stmt) override;
    void visitForStmt(const ForStmt& stmt) override;
    // ExecutorUnit 담당자가 구현할 예정인 미구현 문장. 현재는 실행 시 예외를 던진다.
    void visitFunctionStmt(const FunctionStmt& stmt) override;
    void visitReturnStmt(const ReturnStmt& stmt) override;

    void visitLiteralExpr(const LiteralExpr& expr) override;
    void visitVariableExpr(const VariableExpr& expr) override;
    void visitAssignExpr(const AssignExpr& expr) override;
    void visitBinaryExpr(const BinaryExpr& expr) override;
    void visitUnaryExpr(const UnaryExpr& expr) override;
    void visitGroupingExpr(const GroupingExpr& expr) override;
    void visitLogicalExpr(const LogicalExpr& expr) override;
    // ExecutorUnit 담당자가 구현할 예정인 미구현 표현식. 현재는 평가 시 예외를 던진다.
    void visitCallExpr(const CallExpr& expr) override;

    shared_ptr<Environment> globals;
    shared_ptr<Environment> environment;

    Value evaluation_result;
    bool has_evaluation_result = false;
};
