#include "CodeFabFunction.h"

#include "CodeFabClass.h"
#include "CodeFabInstance.h"
#include "Interpreter.h"

#include "../AssemblerUnit/Parser/Expression.h"
#include "../AssemblerUnit/Parser/Statement.h"
#include "../AssemblerUnit/Tokenizer/Token.h"

#include <gmock/gmock.h>

#include <memory>
#include <unordered_map>
#include <variant>
#include <vector>

using std::get;
using std::make_shared;
using std::make_unique;
using std::monostate;
using std::move;
using std::shared_ptr;
using std::unordered_map;
using std::vector;

namespace {
    Token identifier(const string& name) {
        return Token(TokenType::IDENTIFIER, name, monostate{}, 1);
    }
}

class CodeFabFunctionTestFixture : public testing::Test {
public:
    Interpreter interpreter;
    shared_ptr<Environment> globals = make_shared<Environment>();
};

TEST_F(CodeFabFunctionTestFixture, ArityMatchesParameterCount) {
    vector<Token> params{ identifier("a"), identifier("b") };
    FunctionStmt declaration(identifier("add"), params, make_unique<BlockStmt>());

    CodeFabFunction function(&declaration, globals);

    EXPECT_EQ(function.arity(), 2);
}

TEST_F(CodeFabFunctionTestFixture, CallReturnsValueFromReturnStmt) {
    vector<Token> params{ identifier("a") };
    auto body = make_unique<BlockStmt>();
    body->addStatement(make_unique<ReturnStmt>(identifier("return"), make_unique<VariableExpr>(identifier("a"))));
    FunctionStmt declaration(identifier("identity"), params, move(body));

    CodeFabFunction function(&declaration, globals);
    interpreter.resolve(&declaration);
    Value result = function.call(interpreter, { 3.0 });

    EXPECT_EQ(get<double>(result), 3.0);
}

TEST_F(CodeFabFunctionTestFixture, CallWithoutReturnStmtYieldsNil) {
    FunctionStmt declaration(identifier("noop"), {}, make_unique<BlockStmt>());

    CodeFabFunction function(&declaration, globals);
    Value result = function.call(interpreter, {});

    EXPECT_TRUE(std::holds_alternative<monostate>(result));
}

TEST_F(CodeFabFunctionTestFixture, InitializerAlwaysReturnsBoundInstanceRegardlessOfReturnStmt) {
    FunctionStmt declaration(identifier("init"), {}, make_unique<BlockStmt>());
    CodeFabFunction function(&declaration, globals, /*is_initializer=*/true);

    auto klass = make_shared<CodeFabClass>("Robot", nullptr, unordered_map<string, shared_ptr<CodeFabFunction>>{});
    auto instance = make_shared<CodeFabInstance>(klass);

    shared_ptr<CodeFabFunction> bound = function.bind(instance);
    Value result = bound->call(interpreter, {});

    auto* returned = std::get_if<shared_ptr<Callable>>(&result);
    ASSERT_NE(returned, nullptr);
    EXPECT_EQ(std::dynamic_pointer_cast<CodeFabInstance>(*returned), instance);
}

TEST_F(CodeFabFunctionTestFixture, BindCreatesCopySharingSameDeclarationWithThisDefined) {
    auto body = make_unique<BlockStmt>();
    body->addStatement(make_unique<ReturnStmt>(identifier("return"), make_unique<ThisExpr>(identifier("this"))));
    FunctionStmt declaration(identifier("getSelf"), {}, move(body));
    CodeFabFunction function(&declaration, globals);

    auto klass = make_shared<CodeFabClass>("Robot", nullptr, unordered_map<string, shared_ptr<CodeFabFunction>>{});
    auto instance = make_shared<CodeFabInstance>(klass);

    Value result = function.bind(instance)->call(interpreter, {});

    auto* returned = std::get_if<shared_ptr<Callable>>(&result);
    ASSERT_NE(returned, nullptr);
    EXPECT_EQ(std::dynamic_pointer_cast<CodeFabInstance>(*returned), instance);
}
