#include "Environment.h"
#include "../RuntimeError.h"

#include <gmock/gmock.h>

class EnvironmentTestFixture : public testing::Test {
public:
    Environment env;
};

// 동일 var 이름이 중복 들어오는 것은 checker unit에서 error 처리하므로 실제 발생 확률 없음.

TEST_F(EnvironmentTestFixture, DefinedVariableIsReadable)
{
    env.define("a", 3.0);

    auto value = env.get("a");

    EXPECT_EQ(std::get<double>(value), 3.0);
}

TEST_F(EnvironmentTestFixture, DefineAssignGet)
{
    env.define("a", 3.0);
    env.assign("a", 2.0);
    auto value = env.get("a");

    EXPECT_EQ(std::get<double>(value), 2.0);
}

TEST_F(EnvironmentTestFixture, UndefinedVariableThrowException)
{
    EXPECT_THROW(env.get("x"), RuntimeError);
}

TEST_F(EnvironmentTestFixture, AssignToUndefinedThrowException)
{
    EXPECT_THROW(env.assign("x", 1.0), RuntimeError);
}

TEST_F(EnvironmentTestFixture, AssignToExistingVariableUpdatesValue)
{
    env.define("a", 1.0);

    env.assign("a", 2.0);

    EXPECT_EQ(std::get<double>(env.get("a")), 2.0);
}

class NestedEnvironmentTestFixture : public testing::Test {
public:
    Environment outer;
    Environment inner{ &outer };
};

TEST_F(NestedEnvironmentTestFixture, GetFindsVariableInEnclosingScope)
{
    outer.define("a", 3.0);

    auto value = inner.get("a");

    EXPECT_EQ(std::get<double>(value), 3.0);
}

TEST_F(NestedEnvironmentTestFixture, AssignUpdatesVariableInEnclosingScope)
{
    outer.define("a", 3.0);

    inner.assign("a", 5.0);

    EXPECT_EQ(std::get<double>(outer.get("a")), 5.0);
}

TEST_F(NestedEnvironmentTestFixture, GetUndefinedVariableThrowsThroughChain)
{
    EXPECT_THROW(inner.get("x"), RuntimeError);
}