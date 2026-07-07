#include "Environment.h"
#include "../RuntimeError.h"

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

TEST_F(EnvironmentTestFixture, UndefinedVariableThrowException)
{
    EXPECT_THROW(env.get("x"), RuntimeError);
}

TEST_F(EnvironmentTestFixture, AssignToUndefinedThrowException)
{
    EXPECT_THROW(env.assign("x", 1.0), RuntimeError);
}