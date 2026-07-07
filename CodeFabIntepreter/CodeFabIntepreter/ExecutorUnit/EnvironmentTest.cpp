#include "Environment.h"

#include <gmock/gmock.h>

TEST(EnvironmentTest, DefinedVariableIsReadable)
{
    Environment env;
    env.define("a", 3.0);

    auto value = env.get("a");

    ASSERT_TRUE(value.has_value());
    EXPECT_EQ(std::get<double>(*value), 3.0);
}

TEST(EnvironmentTest, UndefinedVariableReturnsNullopt)
{
    Environment env;

    EXPECT_FALSE(env.get("x").has_value());
}

TEST(EnvironmentTest, AssignToUndefinedVariableFails)
{
    Environment env;

    EXPECT_FALSE(env.assign("x", 1.0));
}