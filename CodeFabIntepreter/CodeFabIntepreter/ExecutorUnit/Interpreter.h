#pragma once

#include "ConstantFolder.h"
#include "Environment.h"
#include "LineResolver.h"
#include "Resolver.h"

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
#ifdef _DEBUG
    explicit Interpreter(
        function<bool(const string&)> file_exists = defaultFileExists,
        function<string(const string&)> read_source = defaultReadSource);
#else
    Interpreter();
#endif

#ifdef _DEBUG
    void interpret(const vector<unique_ptr<Statement>>& statements) override;
#else
    void interpret(const vector<unique_ptr<Statement>>& statements);
#endif
    Value evaluate(const Expression* expr);
    Value getVariableValue(const string& name) const;

    // 문장을 실행하기 직전마다 호출되는 훅을 등록한다. 디버그 모드가 Stmt 단위로
    // stepping/breakpoint를 지원하는 데 사용하며, 등록하지 않으면 아무 영향이 없다.
#ifdef _DEBUG
    void setBeforeStatementHook(function<void(int line)> hook) override;
#else
    void setBeforeStatementHook(function<void(int line)> hook);
#endif

    // 현재(가장 안쪽) 스코프부터 전역까지 거슬러 올라가며 보이는 모든 변수를
    // 열거한다. watch/inspect가 디버그 정지 시점에 값을 조회하는 데 사용한다.
#ifdef _DEBUG
    vector<VariableSnapshot> inspectVariables() const override;
#else
    vector<VariableSnapshot> inspectVariables() const;
#endif

    // CodeFabFunction이 함수 호출마다 파라미터 스코프를 새로 만들어 몸통을
    // 실행할 때 사용하는 진입점.
    void executeBlockWithEnvironment(const BlockStmt* block, shared_ptr<Environment> new_environment);

private:
    void execute(Statement* stmt);
    int resolveStatementLine(const Statement* stmt) const;
    void executeBlockStmt(BlockStmt* block);
    void executeVarDeclareStmt(VarDeclareStmt* var_decl);
    void executeExpressionStmt(ExpressionStmt* stmt);
    void executePrintStmt(PrintStmt* stmt);
    void executeIfStmt(IfStmt* if_stmt);
    void executeForStmt(ForStmt* for_stmt);
    void executeFunctionStmt(FunctionStmt* stmt);
    void executeReturnStmt(ReturnStmt* stmt);
    void executeClassStmt(ClassStmt* stmt);
    void executeImportStmt(ImportStmt* stmt);

    Value evaluateLiteralExpr(const LiteralExpr* literal);
    Value evaluateVariableExpr(const VariableExpr* variable);
    Value evaluateAssignExpr(const AssignExpr& expr);
    Value evaluateBinaryExpr(const BinaryExpr& expr);
    void ensureNumberOperands(const Token& op, const Value& left, const Value& right) const;
    Value evaluateUnaryExpr(const UnaryExpr& expr);
    Value evaluateLogicalExpr(const LogicalExpr& expr);
    Value evaluateCallExpr(const CallExpr& expr);
    Value evaluateGetExpr(const GetExpr& expr);
    Value evaluateSetExpr(const SetExpr& expr);
    Value evaluateThisExpr(const ThisExpr& expr);
    Value evaluateSuperExpr(const SuperExpr& expr);
    Value evaluateInstanceOfExpr(const InstanceOfExpr& expr);
    Value evaluateArrayExpr(const ArrayExpr& expr);
    Value evaluateIndexExpr(const IndexExpr& expr);
    Value evaluateIndexSetExpr(const IndexSetExpr& expr);
    int resolveLine(const Expression* expr) const;

    // resolver가 expr에 대해 계산해둔 거리가 있으면 그 스코프로 곧장 접근하고,
    // 없으면(전역 변수) 기존 이름 기반 체인 탐색으로 대체 처리한다.
    Value lookUpVariable(const Token& name, const Expression* expr);

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

    shared_ptr<Environment> globals;
    shared_ptr<Environment> environment;

    Value evaluation_result;
    bool has_evaluation_result = false;

    function<void(int line)> before_statement_hook;

    // resolveLine/resolveStatementLine이 위임하는 순수 조회용 Visitor. 스스로
    // 관찰 가능한 상태를 갖지 않으므로 const 메서드에서도 안전하게 사용한다.
    mutable LineResolver line_resolver;

    // interpret()이 실행 직전에 호출해, 변수/this/super 참조마다 몇 단계 바깥
    // 스코프에서 선언됐는지 미리 계산해두는 Visitor(정적 바인딩). lookUpVariable과
    // evaluateAssignExpr은 여기 계산된 거리가 있으면 Environment::getAt/assignAt으로
    // 곧장 접근하고, 없으면(전역 변수) 기존 이름 기반 체인 탐색으로 대체 처리한다.
    Resolver resolver;

    // interpret()이 실행 직전에 호출해, 리터럴로만 이뤄진 연산식의 값을 미리
    // 계산해두는 Visitor. evaluate()는 여기 값이 있으면 재계산 없이 즉시 돌려준다.
    ConstantFolder constant_folder;

    static bool defaultFileExists(const string& path);
    static string defaultReadSource(const string& path);

    function<bool(const string&)> file_exists;
    function<string(const string&)> read_source;

    // executeImportStmt가 재귀적으로 interpret()을 호출하는 동안 현재 import
    // 경로들을 쌓아두는 스택. 이미 스택에 있는 경로를 다시 import하려 하면
    // 순환 import로 판단해 예외를 던진다.
    vector<string> import_stack;

    // import된 파일의 문장(AST)을 이 Interpreter가 살아있는 동안 계속 보관한다.
    // CodeFabFacade::retained_statements와 같은 이유로, import된 파일 안의
    // Func/Class 선언이 만드는 클로저가 그 AST를 가리키는 raw pointer를 들고
    // 있기 때문이다.
    vector<vector<unique_ptr<Statement>>> imported_statements;
};
