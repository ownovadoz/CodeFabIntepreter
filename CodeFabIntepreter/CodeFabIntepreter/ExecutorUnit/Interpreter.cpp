#include "Interpreter.h"

#include "CodeFabClass.h"
#include "CodeFabFunction.h"
#include "CodeFabInstance.h"
#include "ReturnSignal.h"
#include "../CodeFabException.h"
#include <iostream>
#include <memory>
#include <string>
#include <unordered_map>
#include <utility>
#include <variant>
using std::cout;
using std::dynamic_pointer_cast;
using std::make_shared;
using std::unordered_map;

Interpreter::Interpreter() : constant_folder(*this)
{
    globals = make_shared<Environment>();
    environment = globals;
}

void Interpreter::interpret(const vector<unique_ptr<Statement>>& statements)
{
    constant_folder.fold(statements);

    for (const auto& statement : statements)
        execute(statement.get());
}

Value Interpreter::getVariableValue(const string& name) const
{
    return environment->get(Token(TokenType::IDENTIFIER, name, Value(), 0));
}

void Interpreter::setBeforeStatementHook(function<void(int line)> hook)
{
    before_statement_hook = std::move(hook);
}

vector<VariableSnapshot> Interpreter::inspectVariables() const
{
    vector<VariableSnapshot> snapshot;

    for (shared_ptr<Environment> scope = environment; scope != nullptr; scope = scope->getEnclosing()) {
        bool is_global = scope->isGlobal();
        for (const auto& [name, value] : scope->getOwnVariables()) {
            snapshot.push_back({ name, value, is_global });
        }
    }

    return snapshot;
}

void Interpreter::execute(Statement* stmt)
{
    if (stmt == nullptr) return;

    // BlockStmt 자신은 컨테이너일 뿐 실행 지점이 아니다. 훅을 여기서도 부르면
    // executeBlockStmt가 이어서 첫 내부 문장에 대해 다시 훅을 불러 같은 줄에서
    // 두 번 멈추게 된다.
    if (before_statement_hook && dynamic_cast<const BlockStmt*>(stmt) == nullptr) {
        before_statement_hook(resolveStatementLine(stmt));
    }

    stmt->accept(*this);
}

int Interpreter::resolveStatementLine(const Statement* stmt) const
{
    return line_resolver.resolve(stmt);
}

void Interpreter::executeBlockStmt(BlockStmt* block)
{
    executeBlockWithEnvironment(block, make_shared<Environment>(environment));
}

void Interpreter::executeBlockWithEnvironment(const BlockStmt* block, shared_ptr<Environment> new_environment)
{
    shared_ptr<Environment> previous = environment;
    environment = move(new_environment);

    try {
        for (const auto& stmt : block->getStatements())
            execute(stmt.get());
    }
    catch (...) {
        environment = previous;
        throw;
    }

    environment = previous;
}

void Interpreter::executeVarDeclareStmt(VarDeclareStmt* var_decl)
{
    Value value;
    if (const Expression* initializer = var_decl->getInitializer()) {
        value = evaluate(initializer);
    }

    environment->define(var_decl->getName().getLexeme(), value);
}

void Interpreter::executeExpressionStmt(ExpressionStmt* stmt)
{
    evaluate(stmt->getExpr());
}

void Interpreter::executePrintStmt(PrintStmt* stmt)
{
    Value value = evaluate(stmt->getExpr());
    cout << stringify(value) << "\n";
}

void Interpreter::executeIfStmt(IfStmt* if_stmt)
{
    if (isTruthy(evaluate(if_stmt->getCondition()))) {
        execute(const_cast<Statement*>(if_stmt->getThenBranch()));
    }
    else {
        execute(const_cast<Statement*>(if_stmt->getElseBranch()));
    }
}

void Interpreter::executeFunctionStmt(FunctionStmt* stmt)
{
    shared_ptr<Callable> function = make_shared<CodeFabFunction>(stmt, environment);
    environment->define(stmt->getName().getLexeme(), Value(function));
}

void Interpreter::executeReturnStmt(ReturnStmt* stmt)
{
    Value value;
    if (stmt->getValue() != nullptr) value = evaluate(stmt->getValue());

    throw ReturnSignal(value);
}

void Interpreter::executeClassStmt(ClassStmt* stmt)
{
    shared_ptr<CodeFabClass> superclass = nullptr;
    if (stmt->getSuperclass() != nullptr) {
        Value superclass_value = evaluate(stmt->getSuperclass());
        shared_ptr<Callable>* callable = std::get_if<shared_ptr<Callable>>(&superclass_value);
        superclass = callable ? dynamic_pointer_cast<CodeFabClass>(*callable) : nullptr;

        if (!superclass)
            throw CodeFabException(stmt->getSuperclass()->getToken(), "클래스가 아닌 대상은 상속할 수 없습니다: '" + stmt->getSuperclass()->getToken().getLexeme() + "'");
    }

    // 상속받은 메서드들이 Super.xxx()로 부모 클래스를 찾을 수 있도록, "super"라는
    // 이름으로 부모 클래스를 바인딩한 환경을 만들어 그 클래스의 모든 메서드가 공유하게 한다.
    shared_ptr<Environment> method_environment = environment;
    if (superclass) {
        method_environment = make_shared<Environment>(environment);
        method_environment->define("super", superclass);
    }

    unordered_map<string, shared_ptr<CodeFabFunction>> methods;
    for (const auto& method : stmt->getMethods())
        methods[method->getName().getLexeme()] = make_shared<CodeFabFunction>(method.get(), method_environment, method->isInitializer());

    auto klass = make_shared<CodeFabClass>(stmt->getName().getLexeme(), superclass, move(methods));
    environment->define(stmt->getName().getLexeme(), klass);
}

void Interpreter::executeForStmt(ForStmt* for_stmt)
{
    shared_ptr<Environment> previous = environment;
    environment = make_shared<Environment>(previous);

    if (for_stmt->getInit() != nullptr) {
        executeVarDeclareStmt(const_cast<VarDeclareStmt*>(for_stmt->getInit()));
    }

    while (for_stmt->getCondition() == nullptr || isTruthy(evaluate(for_stmt->getCondition()))) {
        execute(const_cast<Statement*>(for_stmt->getBody()));

        if (for_stmt->getIncrement() != nullptr) {
            evaluate(for_stmt->getIncrement());
        }
    }

    environment = previous;
}

Value Interpreter::evaluate(const Expression* expr)
{
    const auto& folded_values = constant_folder.getFoldedValues();
    auto found = folded_values.find(expr);
    if (found != folded_values.end()) return found->second;

    has_evaluation_result = false;
    expr->accept(*this);

    if (!has_evaluation_result) throw CodeFabException(resolveLine(expr), "지원하지 않는 표현식입니다.");

    return evaluation_result;
}

Value Interpreter::evaluateLiteralExpr(const LiteralExpr* literal)
{
    return literal->getToken().getLiteral();
}

Value Interpreter::evaluateVariableExpr(const VariableExpr* variable)
{
    return environment->get(variable->getToken());
}

int Interpreter::resolveLine(const Expression* expr) const
{
    return line_resolver.resolve(expr);
}

void Interpreter::visitExpressionStmt(const ExpressionStmt& stmt)
{
    executeExpressionStmt(const_cast<ExpressionStmt*>(&stmt));
}

void Interpreter::visitIfStmt(const IfStmt& stmt)
{
    executeIfStmt(const_cast<IfStmt*>(&stmt));
}

void Interpreter::visitBlockStmt(const BlockStmt& stmt)
{
    executeBlockStmt(const_cast<BlockStmt*>(&stmt));
}

void Interpreter::visitVarDeclareStmt(const VarDeclareStmt& stmt)
{
    executeVarDeclareStmt(const_cast<VarDeclareStmt*>(&stmt));
}

void Interpreter::visitPrintStmt(const PrintStmt& stmt)
{
    executePrintStmt(const_cast<PrintStmt*>(&stmt));
}

void Interpreter::visitForStmt(const ForStmt& stmt)
{
    executeForStmt(const_cast<ForStmt*>(&stmt));
}

void Interpreter::visitFunctionStmt(const FunctionStmt& stmt)
{
    executeFunctionStmt(const_cast<FunctionStmt*>(&stmt));
}

void Interpreter::visitReturnStmt(const ReturnStmt& stmt)
{
    executeReturnStmt(const_cast<ReturnStmt*>(&stmt));
}

void Interpreter::visitClassStmt(const ClassStmt& stmt)
{
    executeClassStmt(const_cast<ClassStmt*>(&stmt));
}

void Interpreter::visitLiteralExpr(const LiteralExpr& expr)
{
    evaluation_result = evaluateLiteralExpr(&expr);
    has_evaluation_result = true;
}

void Interpreter::visitVariableExpr(const VariableExpr& expr)
{
    evaluation_result = evaluateVariableExpr(&expr);
    has_evaluation_result = true;
}

void Interpreter::visitAssignExpr(const AssignExpr& expr)
{
    evaluation_result = evaluateAssignExpr(expr);
    has_evaluation_result = true;
}

Value Interpreter::evaluateAssignExpr(const AssignExpr& expr)
{
    Value value = evaluate(expr.getValue());
    environment->assign(expr.getIdentifier(), value);
    return value;
}
void Interpreter::visitBinaryExpr(const BinaryExpr& expr)
{
    evaluation_result = evaluateBinaryExpr(expr);
    has_evaluation_result = true;
}

Value Interpreter::evaluateBinaryExpr(const BinaryExpr& expr)
{
    Value left = evaluate(expr.getLeft());
    Value right = evaluate(expr.getRight());
    const Token& op = expr.getOperator();

    switch (op.getType()) {
    case TokenType::PLUS:
        if (isNumber(left) && isNumber(right)) return std::get<double>(left) + std::get<double>(right);
        if (isString(left) && isString(right)) return std::get<string>(left) + std::get<string>(right);
        throw CodeFabException(op, "피연산자는 둘 다 숫자이거나 둘 다 문자열이어야 합니다.");

    case TokenType::MINUS:
        ensureNumberOperands(op, left, right);
        return std::get<double>(left) - std::get<double>(right);

    case TokenType::STAR:
        ensureNumberOperands(op, left, right);
        return std::get<double>(left) * std::get<double>(right);

    case TokenType::SLASH:
        ensureNumberOperands(op, left, right);
        if (std::get<double>(right) == 0.0) throw CodeFabException(op, "0으로 나눌 수 없습니다.");
        return std::get<double>(left) / std::get<double>(right);

    case TokenType::GREATER:
        ensureNumberOperands(op, left, right);
        return std::get<double>(left) > std::get<double>(right);

    case TokenType::LESS:
        ensureNumberOperands(op, left, right);
        return std::get<double>(left) < std::get<double>(right);

    case TokenType::GREATER_EQUAL:
        ensureNumberOperands(op, left, right);
        return std::get<double>(left) >= std::get<double>(right);

    case TokenType::LESS_EQUAL:
        ensureNumberOperands(op, left, right);
        return std::get<double>(left) <= std::get<double>(right);

    case TokenType::EQUAL_EQUAL:
        return left == right;

    case TokenType::BANG_EQUAL:
        return left != right;

    default:
        throw CodeFabException(op, "지원하지 않는 이항 연산자입니다.");
    }
}

void Interpreter::ensureNumberOperands(const Token& op, const Value& left, const Value& right) const
{
    if (isNumber(left) && isNumber(right)) return;

    throw CodeFabException(op, "피연산자는 반드시 숫자여야 합니다.");
}
void Interpreter::visitUnaryExpr(const UnaryExpr& expr)
{
    evaluation_result = evaluateUnaryExpr(expr);
    has_evaluation_result = true;
}

Value Interpreter::evaluateUnaryExpr(const UnaryExpr& expr)
{
    Value operand = evaluate(expr.getExpr());
    const Token& op = expr.getOperator();

    if (op.getType() == TokenType::MINUS) {
        if (!isNumber(operand)) throw CodeFabException(op, "피연산자는 반드시 숫자여야 합니다.");
        return -std::get<double>(operand);
    }

    if (op.getType() == TokenType::BANG) {
        return !isTruthy(operand);
    }

    throw CodeFabException(op, "지원하지 않는 단항 연산자입니다.");
}
void Interpreter::visitGroupingExpr(const GroupingExpr& expr)
{
    evaluation_result = evaluate(expr.getExpr());
    has_evaluation_result = true;
}
void Interpreter::visitLogicalExpr(const LogicalExpr& expr)
{
    evaluation_result = evaluateLogicalExpr(expr);
    has_evaluation_result = true;
}

Value Interpreter::evaluateLogicalExpr(const LogicalExpr& expr)
{
    Value left = evaluate(expr.getLeft());
    const Token& op = expr.getOperator();

    if (op.getType() == TokenType::OR) {
        if (isTruthy(left)) return left;
    }
    else {
        if (!isTruthy(left)) return left;
    }

    return evaluate(expr.getRight());
}

void Interpreter::visitCallExpr(const CallExpr& expr)
{
    evaluation_result = evaluateCallExpr(expr);
    has_evaluation_result = true;
}

Value Interpreter::evaluateCallExpr(const CallExpr& expr)
{
    Value callee = evaluate(expr.getCallee());
    if (!isCallable(callee)) throw CodeFabException(expr.getParen(), "호출할 수 없는 대상입니다.");

    vector<Value> arguments;
    for (const auto& argument : expr.getArguments())
        arguments.push_back(evaluate(argument.get()));

    shared_ptr<Callable> callable = std::get<shared_ptr<Callable>>(callee);

    // 인스턴스도 Callable을 구현하지만(Value가 객체 슬롯을 하나만 가지므로) 실제로
    // 호출할 수 있는 대상은 아니므로, CodeFabInstance::call()에 도달하기 전에 걸러낸다.
    if (dynamic_pointer_cast<CodeFabInstance>(callable))
        throw CodeFabException(expr.getParen(), "호출할 수 없는 대상입니다.");

    if (static_cast<int>(arguments.size()) != callable->arity())
        throw CodeFabException(expr.getParen(), "인자 개수가 일치하지 않습니다. 필요한 개수: "
            + std::to_string(callable->arity()) + ", 전달된 개수: " + std::to_string(arguments.size()));

    return callable->call(*this, arguments);
}

void Interpreter::visitGetExpr(const GetExpr& expr)
{
    evaluation_result = evaluateGetExpr(expr);
    has_evaluation_result = true;
}

Value Interpreter::evaluateGetExpr(const GetExpr& expr)
{
    Value object = evaluate(expr.getObject());
    shared_ptr<Callable>* callable = std::get_if<shared_ptr<Callable>>(&object);
    shared_ptr<CodeFabInstance> instance = callable ? dynamic_pointer_cast<CodeFabInstance>(*callable) : nullptr;

    if (!instance) throw CodeFabException(expr.getName(), "인스턴스만 필드에 접근할 수 있습니다.");

    return instance->get(expr.getName());
}

void Interpreter::visitSetExpr(const SetExpr& expr)
{
    evaluation_result = evaluateSetExpr(expr);
    has_evaluation_result = true;
}

Value Interpreter::evaluateSetExpr(const SetExpr& expr)
{
    Value object = evaluate(expr.getObject());
    shared_ptr<Callable>* callable = std::get_if<shared_ptr<Callable>>(&object);
    shared_ptr<CodeFabInstance> instance = callable ? dynamic_pointer_cast<CodeFabInstance>(*callable) : nullptr;

    if (!instance) throw CodeFabException(expr.getName(), "인스턴스만 필드에 접근할 수 있습니다.");

    Value value = evaluate(expr.getValue());
    instance->set(expr.getName(), value);
    return value;
}

void Interpreter::visitThisExpr(const ThisExpr& expr)
{
    evaluation_result = evaluateThisExpr(expr);
    has_evaluation_result = true;
}

Value Interpreter::evaluateThisExpr(const ThisExpr& expr)
{
    return environment->get(Token(TokenType::IDENTIFIER, "this", Value(), expr.getKeyword().getLine()));
}

void Interpreter::visitSuperExpr(const SuperExpr& expr)
{
    evaluation_result = evaluateSuperExpr(expr);
    has_evaluation_result = true;
}

Value Interpreter::evaluateSuperExpr(const SuperExpr& expr)
{
    int line = expr.getKeyword().getLine();

    Value super_value = environment->get(Token(TokenType::IDENTIFIER, "super", Value(), line));
    shared_ptr<Callable>* super_callable = std::get_if<shared_ptr<Callable>>(&super_value);
    shared_ptr<CodeFabClass> superclass = super_callable ? dynamic_pointer_cast<CodeFabClass>(*super_callable) : nullptr;

    shared_ptr<CodeFabFunction> method = superclass->findMethod(expr.getMethod().getLexeme());
    if (!method) throw CodeFabException(expr.getMethod(), "존재하지 않는 메서드입니다: '" + expr.getMethod().getLexeme() + "'");

    Value this_value = environment->get(Token(TokenType::IDENTIFIER, "this", Value(), line));
    shared_ptr<Callable>* this_callable = std::get_if<shared_ptr<Callable>>(&this_value);
    shared_ptr<CodeFabInstance> instance = this_callable ? dynamic_pointer_cast<CodeFabInstance>(*this_callable) : nullptr;

    return method->bind(instance);
}

void Interpreter::visitInstanceOfExpr(const InstanceOfExpr& expr)
{
    evaluation_result = evaluateInstanceOfExpr(expr);
    has_evaluation_result = true;
}

Value Interpreter::evaluateInstanceOfExpr(const InstanceOfExpr& expr)
{
    Value object = evaluate(expr.getObject());
    shared_ptr<Callable>* object_callable = std::get_if<shared_ptr<Callable>>(&object);
    shared_ptr<CodeFabInstance> instance = object_callable ? dynamic_pointer_cast<CodeFabInstance>(*object_callable) : nullptr;

    if (!instance) return false;

    Value class_value = environment->get(expr.getClassName());
    shared_ptr<Callable>* class_callable = std::get_if<shared_ptr<Callable>>(&class_value);
    shared_ptr<CodeFabClass> klass = class_callable ? dynamic_pointer_cast<CodeFabClass>(*class_callable) : nullptr;

    if (!klass) throw CodeFabException(expr.getClassName(), "클래스가 아닙니다: '" + expr.getClassName().getLexeme() + "'");

    return instance->getClass()->isSubclassOf(klass.get());
}

void Interpreter::visitArrayExpr(const ArrayExpr& expr)
{
    evaluation_result = evaluateArrayExpr(expr);
    has_evaluation_result = true;
}

Value Interpreter::evaluateArrayExpr(const ArrayExpr& expr)
{
    Value size_value = evaluate(expr.getSize());
    if (!isNumber(size_value))
        throw CodeFabException(resolveLine(expr.getSize()), "배열 크기는 숫자여야 합니다.");

    double size = std::get<double>(size_value);
    if (size < 0 || size != static_cast<double>(static_cast<int>(size)))
        throw CodeFabException(resolveLine(expr.getSize()), "배열 크기는 0 이상의 정수여야 합니다.");

    return make_shared<CodeFabArray>(static_cast<int>(size));
}

void Interpreter::visitIndexExpr(const IndexExpr& expr)
{
    evaluation_result = evaluateIndexExpr(expr);
    has_evaluation_result = true;
}

Value Interpreter::evaluateIndexExpr(const IndexExpr& expr)
{
    Value array_value = evaluate(expr.getArray());
    if (!isArray(array_value))
        throw CodeFabException(resolveLine(expr.getArray()), "배열이 아닌 값에는 인덱스로 접근할 수 없습니다.");

    Value index_value = evaluate(expr.getIndex());
    if (!isNumber(index_value))
        throw CodeFabException(resolveLine(expr.getIndex()), "배열 인덱스는 숫자여야 합니다.");

    shared_ptr<CodeFabArray> array = std::get<shared_ptr<CodeFabArray>>(array_value);
    int index = static_cast<int>(std::get<double>(index_value));

    if (index < 0 || index >= array->size())
        throw CodeFabException(resolveLine(expr.getIndex()), "배열 범위를 벗어났습니다.");

    return array->get(index);
}

void Interpreter::visitIndexSetExpr(const IndexSetExpr& expr)
{
    evaluation_result = evaluateIndexSetExpr(expr);
    has_evaluation_result = true;
}

Value Interpreter::evaluateIndexSetExpr(const IndexSetExpr& expr)
{
    Value array_value = evaluate(expr.getArray());
    if (!isArray(array_value))
        throw CodeFabException(resolveLine(expr.getArray()), "배열이 아닌 값에는 인덱스로 접근할 수 없습니다.");

    Value index_value = evaluate(expr.getIndex());
    if (!isNumber(index_value))
        throw CodeFabException(resolveLine(expr.getIndex()), "배열 인덱스는 숫자여야 합니다.");

    shared_ptr<CodeFabArray> array = std::get<shared_ptr<CodeFabArray>>(array_value);
    int index = static_cast<int>(std::get<double>(index_value));

    if (index < 0 || index >= array->size())
        throw CodeFabException(resolveLine(expr.getIndex()), "배열 범위를 벗어났습니다.");

    Value value = evaluate(expr.getValue());
    array->set(index, value);

    return value;
}
