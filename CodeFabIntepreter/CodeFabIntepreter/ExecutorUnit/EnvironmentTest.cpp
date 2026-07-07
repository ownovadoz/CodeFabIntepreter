#include "Environment.h"

#include <gmock/gmock.h>

class EnvironmentTestFixture : public testing::Test {
public:
    Environment env;
};

TEST_F(EnvironmentTestFixture, DefinedVariableIsReadable)
{
    env.define("a", 3.0);

    auto value = env.get("a");

    ASSERT_TRUE(value.has_value());
    EXPECT_EQ(std::get<double>(*value), 3.0);
}

TEST_F(EnvironmentTestFixture, UndefinedVariableReturnsNullopt)
{
    EXPECT_FALSE(env.get("x").has_value());
}

TEST_F(EnvironmentTestFixture, AssignToUndefinedVariableFails)
{
    EXPECT_FALSE(env.assign("x", 1.0));
}