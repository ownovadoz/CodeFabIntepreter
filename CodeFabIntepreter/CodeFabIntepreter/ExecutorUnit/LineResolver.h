#pragma once

#include "../AssemblerUnit/Parser/Expression.h"
#include "../AssemblerUnit/Parser/Statement.h"
#include "../Visitor.h"

// AST 노드의 소스 코드 줄 번호를 알아내기 위한 Visitor. 예전에는 Interpreter가
// Expression/Statement의 모든 구체 타입을 dynamic_cast로 하나씩 확인했는데,
// 그러면 새 노드 타입이 추가될 때마다 ExprVisitor/StmtVisitor뿐 아니라 이
// dynamic_cast 체인도 별도로 갱신해야 했다. Visitor를 그대로 재사용하면 컴파일러가
// 누락된 case를 강제로 잡아준다.
class LineResolver : public ExprVisitor, public StmtVisitor {
public:
    int resolve(const Expression* expr);
    int resolve(const Statement* stmt);

private:
    void visitLiteralExpr(const LiteralExpr& expr) override;
    void visitVariableExpr(const VariableExpr& expr) override;
    void visitAssignExpr(const AssignExpr& expr) override;
    void visitBinaryExpr(const BinaryExpr& expr) override;
    void visitUnaryExpr(const UnaryExpr& expr) override;
    void visitGroupingExpr(const GroupingExpr& expr) override;
    void visitLogicalExpr(const LogicalExpr& expr) override;
    void visitCallExpr(const CallExpr& expr) override;
    void visitGetExpr(const GetExpr& expr) override;
    void visitSetExpr(const SetExpr& expr) override;
    void visitThisExpr(const ThisExpr& expr) override;
    void visitSuperExpr(const SuperExpr& expr) override;
    void visitInstanceOfExpr(const InstanceOfExpr& expr) override;
    void visitArrayExpr(const ArrayExpr& expr) override;
    void visitIndexExpr(const IndexExpr& expr) override;
    void visitIndexSetExpr(const IndexSetExpr& expr) override;

    void visitExpressionStmt(const ExpressionStmt& stmt) override;
    void visitIfStmt(const IfStmt& stmt) override;
    void visitBlockStmt(const BlockStmt& stmt) override;
    void visitVarDeclareStmt(const VarDeclareStmt& stmt) override;
    void visitPrintStmt(const PrintStmt& stmt) override;
    void visitForStmt(const ForStmt& stmt) override;
    void visitFunctionStmt(const FunctionStmt& stmt) override;
    void visitReturnStmt(const ReturnStmt& stmt) override;
    void visitClassStmt(const ClassStmt& stmt) override;
    void visitImportStmt(const ImportStmt& stmt) override;

    int line = 0;
};
