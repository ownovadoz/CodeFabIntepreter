#pragma once

#include "../AssemblerUnit/Parser/Expression.h"
#include "../AssemblerUnit/Parser/Statement.h"
#include "../Visitor.h"

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

using std::string;
using std::unique_ptr;
using std::unordered_map;
using std::vector;

// 변수 접근에 필요한 스코프 이동 횟수(거리)를 실행 전에 미리 계산해두는 Visitor.
// Interpreter가 실제로 Environment를 몇 겹 만드는지와 정확히 같은 시점에만 스코프를
// 열고 닫아야, 여기서 계산한 거리가 런타임과 어긋나지 않는다. Checker도 스코프
// 추적을 하지만 그 목적은 중복 선언/자기참조 검출이라 IfStmt 분기마다 항상 스코프를
// 여는 등 Interpreter의 실제 Environment 생성 시점과 다르므로, 이 클래스는 Checker의
// 스코프 로직을 그대로 재사용할 수 없다.
class ScopeResolver : public ExprVisitor, public StmtVisitor {
public:
    void resolve(const vector<unique_ptr<Statement>>& statements);

    // CodeFabFunction/CodeFabClass를 Interpreter::interpret() 없이 직접 만들어
    // call()/bind()를 호출하는 경우(단위 테스트에서 흔함)에도 대상 선언 하나만
    // 미리 훑어둘 수 있도록 raw pointer 버전도 제공한다.
    void resolve(const Statement* stmt);

    // VariableExpr/AssignExpr/ThisExpr/SuperExpr에서만 채워진다. 못 찾으면
    // 전역 변수로 간주하고 아무 항목도 남기지 않는다(Interpreter는 이 경우
    // globals에서 바로 조회한다).
    const unordered_map<const Expression*, int>& getLocals() const { return locals; }

private:
    void beginScope();
    void endScope();
    void declareName(const string& name);
    void declare(const Token& name);
    void resolveLocal(const Expression& expr, const string& name);

    void resolveStmt(const Statement* stmt);
    void resolveExpr(const Expression* expr);
    void resolveFunctionBody(const FunctionStmt& stmt);

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

    // 전역 스코프는 스택에 올리지 않는다. 스택이 비어 있으면 전역 변수라는 뜻이다.
    vector<unordered_map<string, bool>> scopes;
    unordered_map<const Expression*, int> locals;
};
