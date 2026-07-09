#pragma once

#include "Environment.h"

#include "../AssemblerUnit/Parser/Expression.h"
#include "../AssemblerUnit/Parser/Statement.h"
#include "../AssemblerUnit/Tokenizer/Value.h"
#include "../InterfaceForCodeFabTest.h"
#include "../Visitor.h"

#include <functional>
#include <memory>
#include <string>
#include <vector>

using std::function;
using std::string;
using std::shared_ptr;
using std::unique_ptr;
using std::vector;

#ifdef _DEBUG
class Interpreter : public IExecutor, public ExprVisitor, public StmtVisitor
#else
class Interpreter : public ExprVisitor, public StmtVisitor
#endif
{
public:
    Interpreter();

#ifdef _DEBUG
    void interpret(const vector<unique_ptr<Statement>>& statements) override;
#else
    void interpret(const vector<unique_ptr<Statement>>& statements);
#endif
    Value evaluate(const Expression* expr);
    Value getVariableValue(const string& name) const;

    // 문장을 실행하기 직전마다 호출되는 훅을 등록한다. 디버그 모드가 Stmt 단위로
    // stepping/breakpoint를 지원하는 데 사용하며, 등록하지 않으면 아무 영향이 없다.
    void setBeforeStatementHook(function<void(int line)> hook);

private:
    void execute(Statement* stmt);
    int resolveStatementLine(const Statement* stmt) const;
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

    function<void(int line)> before_statement_hook;
};
