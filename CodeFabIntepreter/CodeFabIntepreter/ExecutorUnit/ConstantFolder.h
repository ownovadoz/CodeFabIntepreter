#pragma once

#include "../AssemblerUnit/Parser/Expression.h"
#include "../AssemblerUnit/Parser/Statement.h"
#include "../AssemblerUnit/Tokenizer/Value.h"
#include "../Visitor.h"

#include <memory>
#include <unordered_map>
#include <vector>

using std::unique_ptr;
using std::unordered_map;
using std::vector;

class Interpreter;

// 리터럴로만 이뤄진 연산식(+,-,*,/,비교, and/or, 단항, 그룹)의 값을 실행 전에
// 한 번만 계산해두는 Visitor. Interpreter::evaluate()는 여기서 계산해둔 값이
// 있으면 그 값을 즉시 돌려주고, 실행될 때마다 같은 연산을 다시 계산하지 않는다.
class ConstantFolder : public ExprVisitor, public StmtVisitor {
public:
    explicit ConstantFolder(Interpreter& interpreter) : interpreter(interpreter) {}

    void fold(const vector<unique_ptr<Statement>>& statements);

    const unordered_map<const Expression*, Value>& getFoldedValues() const { return folded; }

private:
    bool isConstant(const Expression* expr) const;
    void tryFold(const Expression& expr);

    void resolveStmt(const Statement* stmt);
    void resolveExpr(const Expression* expr);

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

    Interpreter& interpreter;
    unordered_map<const Expression*, Value> folded;
};
