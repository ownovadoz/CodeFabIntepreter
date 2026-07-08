#pragma once

#include "Environment.h"

#include "../AssemblerUnit/Parser/Expression.h"
#include "../AssemblerUnit/Parser/Statement.h"
#include "../AssemblerUnit/Tokenizer/Value.h"
#include "../Visitor.h"

#include <string>

using std::string;

class Interpreter : public StmtVisitor, public ExprVisitor {
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

    void visitExpressionStmt(ExpressionStmt& stmt) override;
    void visitIfStmt(IfStmt& stmt) override;
    void visitBlockStmt(BlockStmt& stmt) override;
    void visitVarDeclareStmt(VarDeclareStmt& stmt) override;
    void visitPrintStmt(PrintStmt& stmt) override;
    void visitForStmt(ForStmt& stmt) override;
    void visitFunctionDeclStmt(FunctionDeclStmt& stmt) override;
    void visitReturnStmt(ReturnStmt& stmt) override;
    void visitClassDeclStmt(ClassDeclStmt& stmt) override;
    void visitImportStmt(ImportStmt& stmt) override;

    void visitLiteralExpr(const LiteralExpr& expr) override;
    void visitVariableExpr(const VariableExpr& expr) override;
    void visitAssignExpr(const AssignExpr& expr) override;
    void visitBinaryExpr(const BinaryExpr& expr) override;
    void visitUnaryExpr(const UnaryExpr& expr) override;
    void visitGroupingExpr(const GroupingExpr& expr) override;
    void visitLogicalExpr(const LogicalExpr& expr) override;
    void visitArrayExpr(const ArrayExpr& expr) override;
    void visitIndexExpr(const IndexExpr& expr) override;
    void visitIndexSetExpr(const IndexSetExpr& expr) override;
    void visitCallExpr(const CallExpr& expr) override;
    void visitGetExpr(const GetExpr& expr) override;
    void visitSetExpr(const SetExpr& expr) override;
    void visitThisExpr(const ThisExpr& expr) override;
    void visitSuperExpr(const SuperExpr& expr) override;
    void visitInstanceOfExpr(const InstanceOfExpr& expr) override;

    Environment global_environment;
    Environment* current_environment;
    Value evaluation_result;
    bool has_evaluation_result = false;
};
