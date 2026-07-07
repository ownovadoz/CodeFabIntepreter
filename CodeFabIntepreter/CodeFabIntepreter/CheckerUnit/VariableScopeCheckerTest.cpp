#include <gmock/gmock.h>

#include "VariableScopeChecker.h"

class VariableScopeCheckerTest : public ::testing::Test {
protected:
    void SetUp() override { checker.enterScope(); }

    VariableScopeChecker checker;
};

TEST_F(VariableScopeCheckerTest, DeclaringNewVariableSucceeds) {
    EXPECT_FALSE(checker.declareVariable("a", {}).hasError);
}

TEST_F(VariableScopeCheckerTest, DuplicateDeclarationInSameScopeFails) {
    checker.declareVariable("a", {});

    EXPECT_TRUE(checker.declareVariable("a", {}).hasError);
}

TEST_F(VariableScopeCheckerTest, SameNameInNestedScopeSucceeds) {
    checker.declareVariable("a", {});
    checker.enterScope();

    EXPECT_FALSE(checker.declareVariable("a", {}).hasError);
}

TEST_F(VariableScopeCheckerTest, RedeclaringAfterScopeExitSucceeds) {
    checker.enterScope();
    checker.declareVariable("a", {});
    checker.exitScope();

    EXPECT_FALSE(checker.declareVariable("a", {}).hasError);
}

TEST_F(VariableScopeCheckerTest, SelfReferenceInInitializerFails) {
    EXPECT_TRUE(checker.declareVariable("a", { "a" }).hasError);
}

TEST_F(VariableScopeCheckerTest, ReferencingOtherVariableInInitializerSucceeds) {
    checker.declareVariable("b", {});

    EXPECT_FALSE(checker.declareVariable("a", { "b" }).hasError);
}