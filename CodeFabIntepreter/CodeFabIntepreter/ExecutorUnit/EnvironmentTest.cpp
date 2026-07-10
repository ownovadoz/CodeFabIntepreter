#include "Environment.h"
#include "../CodeFabException.h"

#include <gmock/gmock.h>

#include <memory>

using std::make_shared;
using std::shared_ptr;

namespace {
Token identifierToken(const string& name)
{
    return Token(TokenType::IDENTIFIER, name, Value(), 1);
}
}

class EnvironmentTestFixture : public testing::Test {
public:
    Environment env;
};

TEST_F(EnvironmentTestFixture, DefinedVariableIsReadable)
{
    env.define("a", 3.0);

    auto value = env.get(identifierToken("a"));

    EXPECT_EQ(std::get<double>(value), 3.0);
}

TEST_F(EnvironmentTestFixture, DefineAssignGet)
{
    env.define("a", 3.0);
    env.assign(identifierToken("a"), 2.0);
    auto value = env.get(identifierToken("a"));

    EXPECT_EQ(std::get<double>(value), 2.0);
}

TEST_F(EnvironmentTestFixture, UndefinedVariableThrowException)
{
    EXPECT_THROW(env.get(identifierToken("x")), CodeFabException);
}

TEST_F(EnvironmentTestFixture, AssignToUndefinedThrowException)
{
    EXPECT_THROW(env.assign(identifierToken("x"), 1.0), CodeFabException);
}

TEST_F(EnvironmentTestFixture, AssignToExistingVariableUpdatesValue)
{
    env.define("a", 1.0);

    env.assign(identifierToken("a"), 2.0);

    EXPECT_EQ(std::get<double>(env.get(identifierToken("a"))), 2.0);
}

TEST_F(EnvironmentTestFixture, DefaultConstructedEnvironmentIsGlobal)
{
    EXPECT_TRUE(env.isGlobal());
    EXPECT_EQ(env.getEnclosing(), nullptr);
}

TEST_F(EnvironmentTestFixture, GetOwnVariablesOnlyReturnsVariablesDefinedDirectlyHere)
{
    env.define("a", 1.0);
    env.define("b", 2.0);

    EXPECT_EQ(env.getOwnVariables().size(), 2u);
    EXPECT_EQ(std::get<double>(env.getOwnVariables().at("a")), 1.0);
    EXPECT_EQ(std::get<double>(env.getOwnVariables().at("b")), 2.0);
}

class NestedEnvironmentTestFixture : public testing::Test {
public:
    shared_ptr<Environment> outer = make_shared<Environment>();
    shared_ptr<Environment> inner = make_shared<Environment>(outer);
};

TEST_F(NestedEnvironmentTestFixture, GetFindsVariableInEnclosingScope)
{
    outer->define("a", 3.0);

    auto value = inner->get(identifierToken("a"));

    EXPECT_EQ(std::get<double>(value), 3.0);
}

TEST_F(NestedEnvironmentTestFixture, AssignUpdatesVariableInEnclosingScope)
{
    outer->define("a", 3.0);

    inner->assign(identifierToken("a"), 5.0);

    EXPECT_EQ(std::get<double>(outer->get(identifierToken("a"))), 5.0);
}

TEST_F(NestedEnvironmentTestFixture, DefineShadowsVariableInEnclosingScope)
{
    outer->define("a", 3.0);

    inner->define("a", 5.0);

    EXPECT_EQ(std::get<double>(inner->get(identifierToken("a"))), 5.0);
}

TEST_F(NestedEnvironmentTestFixture, GetUndefinedVariableThrowsThroughChain)
{
    EXPECT_THROW(inner->get(identifierToken("x")), CodeFabException);
}

TEST_F(NestedEnvironmentTestFixture, AssignUndefinedVariableThrowsThroughChain)
{
    EXPECT_THROW(inner->assign(identifierToken("x"), 1.0), CodeFabException);
}

TEST_F(NestedEnvironmentTestFixture, InnerEnvironmentIsNotGlobalAndPointsToOuter)
{
    EXPECT_FALSE(inner->isGlobal());
    EXPECT_EQ(inner->getEnclosing(), outer);
    EXPECT_TRUE(outer->isGlobal());
}

TEST_F(NestedEnvironmentTestFixture, GetOwnVariablesDoesNotSeeEnclosingScopeVariables)
{
    outer->define("a", 3.0);
    inner->define("b", 4.0);

    EXPECT_EQ(inner->getOwnVariables().size(), 1u);
    EXPECT_EQ(std::get<double>(inner->getOwnVariables().at("b")), 4.0);
}
