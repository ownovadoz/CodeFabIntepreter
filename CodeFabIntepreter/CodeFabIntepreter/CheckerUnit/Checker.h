#pragma once

#include "../AssemblerUnit/Parser/Statement.h"
#include "../InterfaceForCodeFabTest.h"
#include "../Visitor.h"

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

using std::string;
using std::unique_ptr;
using std::unordered_map;
using std::vector;

#ifdef _DEBUG
class Checker : public IChecker, public ExprVisitor, public StmtVisitor
#else
class Checker : public ExprVisitor, public StmtVisitor
#endif
{
public:
    Checker();

    class ScopeGuard
    {
    public:
        explicit ScopeGuard(Checker& checker) : checker(checker) { checker.beginScope(); }
        ~ScopeGuard() { checker.endScope(); }

        ScopeGuard(const ScopeGuard&) = delete;
        ScopeGuard& operator=(const ScopeGuard&) = delete;

    private:
        Checker& checker;
    };

    class FunctionGuard
    {
    public:
        explicit FunctionGuard(Checker& checker, bool is_initializer = false)
            : checker(checker), previous_in_initializer(checker.in_initializer), previous_loop_depth(checker.loop_depth)
        {
            checker.function_depth++;
            checker.in_initializer = is_initializer;
            // 함수 몸통은 그 함수가 반복문 안에서 선언됐더라도 매 반복마다 실행되는
            // 것이 아니라 호출될 때만 실행되므로, 함수 경계를 넘으면 반복문
            // 문맥을 리셋한다 - 반복문 안에서 선언된 함수 안의 import는 허용된다.
            checker.loop_depth = 0;
        }
        ~FunctionGuard()
        {
            checker.function_depth--;
            checker.in_initializer = previous_in_initializer;
            checker.loop_depth = previous_loop_depth;
        }

        FunctionGuard(const FunctionGuard&) = delete;
        FunctionGuard& operator=(const FunctionGuard&) = delete;

    private:
        Checker& checker;
        bool previous_in_initializer;
        int previous_loop_depth;
    };

    // 반복문 몸통을 검사하는 동안 import 사용 가능 여부를 추적한다.
    class LoopGuard
    {
    public:
        explicit LoopGuard(Checker& checker) : checker(checker) { checker.loop_depth++; }
        ~LoopGuard() { checker.loop_depth--; }

        LoopGuard(const LoopGuard&) = delete;
        LoopGuard& operator=(const LoopGuard&) = delete;

    private:
        Checker& checker;
    };

    // 클래스 본문을 검사하는 동안 this/Super 사용 가능 여부를 추적한다.
    class ClassGuard
    {
    public:
        ClassGuard(Checker& checker, bool has_superclass) : checker(checker)
        {
            checker.class_stack.push_back(ClassContext{ has_superclass });
        }
        ~ClassGuard() { checker.class_stack.pop_back(); }

        ClassGuard(const ClassGuard&) = delete;
        ClassGuard& operator=(const ClassGuard&) = delete;

    private:
        Checker& checker;
    };

#ifdef _DEBUG
    void check(const vector<unique_ptr<Statement>>& statements) override;
#else
    void check(const vector<unique_ptr<Statement>>& statements);
#endif

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

private:
    struct ClassContext
    {
        bool has_superclass;
    };

    void beginScope();
    void endScope();

    void declare(const Token& name);
    void define(const Token& name);

    void resolveStmt(const Statement* stmt);
    void resolveStmtInNewScope(const Statement* stmt);
    void resolveExpr(const Expression* expr);
    void resolveFunction(const FunctionStmt& stmt, bool is_initializer = false);

    // 스코프 내 각 변수 이름은 declare 시 false(선언됨, 아직 정의되지 않음)로,
    // define 시 true(정의 완료)로 표시된다. 초기화식을 검사하는 시점에는
    // 아직 정의되지 않은 상태이므로, 이 시점에 같은 스코프에서 자신의 이름이
    // false로 발견되면 자기참조로 판단한다.
    vector<unordered_map<string, bool>> scope_stack;

    // return문이 함수 몸통 내부에서 사용됐는지 판단하기 위한 중첩 함수 개수.
    int function_depth = 0;
    // import가 반복문 몸통 안에서 곧바로 쓰였는지 판단하기 위한 중첩 반복문 개수.
    // 함수 경계를 넘으면(FunctionGuard) 리셋된다.
    int loop_depth = 0;
    // 현재 검사 중인 함수가 생성자(init)인지 여부. init 안에서는 return 자체가 금지된다.
    bool in_initializer = false;
    // 현재 검사 중인 클래스(중첩 시 가장 안쪽)의 문맥. this/Super 사용 가능 여부를 검사한다.
    vector<ClassContext> class_stack;
};
