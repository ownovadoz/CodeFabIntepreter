#include "CodeFabClass.h"

#include "CodeFabFunction.h"
#include "CodeFabInstance.h"
#include "Interpreter.h"

#include "../AssemblerUnit/Parser/Expression.h"
#include "../AssemblerUnit/Parser/Statement.h"
#include "../AssemblerUnit/Tokenizer/Token.h"

#include <gmock/gmock.h>

#include <memory>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>

using std::get;
using std::make_shared;
using std::make_unique;
using std::monostate;
using std::move;
using std::shared_ptr;
using std::string;
using std::unique_ptr;
using std::unordered_map;
using std::vector;

namespace {
    Token identifier(const string& name) {
        return Token(TokenType::IDENTIFIER, name, monostate{}, 1);
    }

    shared_ptr<Environment> globalEnvironment() {
        return make_shared<Environment>();
    }
}

class CodeFabClassTestFixture : public testing::Test {
public:
    Interpreter interpreter;
};

TEST_F(CodeFabClassTestFixture, FindMethodReturnsOwnMethodWhenPresent) {
    FunctionStmt move_decl(identifier("move"), {}, make_unique<BlockStmt>());
    auto move_fn = make_shared<CodeFabFunction>(&move_decl, globalEnvironment());

    unordered_map<string, shared_ptr<CodeFabFunction>> methods{ { "move", move_fn } };
    CodeFabClass robot("Robot", nullptr, methods);

    EXPECT_EQ(robot.findMethod("move"), move_fn);
}

TEST_F(CodeFabClassTestFixture, FindMethodFallsBackToSuperclassWhenNotOverridden) {
    FunctionStmt move_decl(identifier("move"), {}, make_unique<BlockStmt>());
    auto move_fn = make_shared<CodeFabFunction>(&move_decl, globalEnvironment());
    auto robot = make_shared<CodeFabClass>("Robot", nullptr, unordered_map<string, shared_ptr<CodeFabFunction>>{ { "move", move_fn } });

    CodeFabClass speed_robot("SpeedRobot", robot, unordered_map<string, shared_ptr<CodeFabFunction>>{});

    EXPECT_EQ(speed_robot.findMethod("move"), move_fn);
}

TEST_F(CodeFabClassTestFixture, FindMethodReturnsNullWhenNowhereInChain) {
    CodeFabClass robot("Robot", nullptr, unordered_map<string, shared_ptr<CodeFabFunction>>{});

    EXPECT_EQ(robot.findMethod("notExist"), nullptr);
}

TEST_F(CodeFabClassTestFixture, IsSubclassOfIsTrueForSelfAndAncestors) {
    auto robot = make_shared<CodeFabClass>("Robot", nullptr, unordered_map<string, shared_ptr<CodeFabFunction>>{});
    auto speed_robot = make_shared<CodeFabClass>("SpeedRobot", robot, unordered_map<string, shared_ptr<CodeFabFunction>>{});
    CodeFabClass unrelated("Cat", nullptr, unordered_map<string, shared_ptr<CodeFabFunction>>{});

    EXPECT_TRUE(speed_robot->isSubclassOf(speed_robot.get()));
    EXPECT_TRUE(speed_robot->isSubclassOf(robot.get()));
    EXPECT_FALSE(speed_robot->isSubclassOf(&unrelated));
}

TEST_F(CodeFabClassTestFixture, ArityDelegatesToInitializerWhenPresent) {
    vector<Token> params{ identifier("name"), identifier("speed") };
    FunctionStmt init_decl(identifier("init"), params, make_unique<BlockStmt>());
    auto init_fn = make_shared<CodeFabFunction>(&init_decl, globalEnvironment(), true);

    CodeFabClass robot("Robot", nullptr, unordered_map<string, shared_ptr<CodeFabFunction>>{ { "init", init_fn } });

    EXPECT_EQ(robot.arity(), 2);
}

TEST_F(CodeFabClassTestFixture, ArityIsZeroWhenNoInitializer) {
    CodeFabClass robot("Robot", nullptr, unordered_map<string, shared_ptr<CodeFabFunction>>{});

    EXPECT_EQ(robot.arity(), 0);
}

TEST_F(CodeFabClassTestFixture, CallCreatesInstanceAndInvokesInitializer) {
    vector<Token> params{ identifier("name") };
    auto body = make_unique<BlockStmt>();
    body->addStatement(make_unique<ExpressionStmt>(
        make_unique<SetExpr>(make_unique<ThisExpr>(identifier("this")), identifier("name"), make_unique<VariableExpr>(identifier("name")))));
    FunctionStmt init_decl(identifier("init"), params, move(body));
    auto init_fn = make_shared<CodeFabFunction>(&init_decl, globalEnvironment(), true);

    auto robot = make_shared<CodeFabClass>("Robot", nullptr, unordered_map<string, shared_ptr<CodeFabFunction>>{ { "init", init_fn } });

    Value result = robot->call(interpreter, { string("Sam") });

    auto* callable = std::get_if<shared_ptr<Callable>>(&result);
    ASSERT_NE(callable, nullptr);
    auto instance = std::dynamic_pointer_cast<CodeFabInstance>(*callable);
    ASSERT_NE(instance, nullptr);
    EXPECT_EQ(get<string>(instance->get(identifier("name"))), "Sam");
}
