#pragma once

#include "../AssemblerUnit/Parser/Expression.h"
#include "../AssemblerUnit/Parser/Statement.h"
#include "../AssemblerUnit/Tokenizer/Token.h"
#include "../Visitor.h"

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

using std::string;
using std::unique_ptr;
using std::unordered_map;
using std::vector;

// interpret() 실행 직전에 AST를 한 번 훑어, 지역 변수(파라미터, this, super 포함)
// 참조마다 몇 단계 바깥 스코프에서 선언됐는지("거리")를 미리 계산해두는 Visitor다.
// Environment::get/assign이 매번 enclosing 체인을 이름으로 훑는 대신, 계산해둔
// 거리로 Environment::getAt/assignAt을 통해 곧장 해당 스코프에 접근할 수 있게 한다.
//
// Checker와 동일한 스코프 스택 방식으로 추적하되(스코프 진입/이탈 시점이 서로
// 대응), 두 가지가 다르다:
//   1. Checker는 중복 선언 검출을 위해 전역 스코프를 인스턴스 생존 기간 내내
//      유지하지만, Resolver는 resolve() 호출(=REPL 한 줄)마다 스코프 스택을
//      빈 상태로 시작한다. 그 호출의 지역 스코프 안에서 찾지 못한 참조는 전역
//      변수로 간주해 거리를 계산해두지 않고, Environment::get/assign의 기존
//      이름 기반 체인 탐색으로 대체 처리된다.
//   2. if/for의 분기·몸통은 그 자체로 새 스코프를 열지 않는다 - Interpreter도
//      해당 문장이 BlockStmt일 때만 새 Environment를 만들기 때문에, 여기서도
//      실제 실행 시점의 Environment 중첩 구조를 그대로 따라간다.
class Resolver : public ExprVisitor, public StmtVisitor {
public:
    class ScopeGuard {
    public:
        explicit ScopeGuard(Resolver& resolver) : resolver(resolver) { resolver.beginScope(); }
        ~ScopeGuard() { resolver.endScope(); }

        ScopeGuard(const ScopeGuard&) = delete;
        ScopeGuard& operator=(const ScopeGuard&) = delete;

    private:
        Resolver& resolver;
    };

    void resolve(const vector<unique_ptr<Statement>>& statements);

    // expr이 지역 변수 참조로 풀렸다면 그 거리를, 아니면(전역이거나 아직 미지원
    // 표현식이면) nullptr을 반환한다.
    const int* getDistance(const Expression* expr) const;

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

private:
    void beginScope();
    void endScope();

    void declare(const Token& name);
    void define(const Token& name);
    void resolveLocal(const Expression& expr, const string& name);

    void resolveStmt(const Statement* stmt);
    void resolveExpr(const Expression* expr);
    void resolveFunction(const FunctionStmt& stmt);

    // Checker::scope_stack과 달리 resolve() 호출마다 비워진 상태로 시작하는
    // 지역 스코프 스택. 각 원소는 그 스코프에서 선언된 이름 -> 정의 완료 여부.
    vector<unordered_map<string, bool>> scopes;

    // VariableExpr/AssignExpr/ThisExpr/SuperExpr/InstanceOfExpr 노드 포인터별로
    // 계산해둔 거리. AST는 CodeFabFacade가 영구 보관하므로 포인터가 안정적이다.
    unordered_map<const Expression*, int> locals;
};
