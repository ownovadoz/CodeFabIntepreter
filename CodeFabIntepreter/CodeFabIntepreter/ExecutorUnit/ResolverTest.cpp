#include "Resolver.h"

#include "../AssemblerUnit/AssemblerUnit.h"
#include "../AssemblerUnit/Parser/Expression.h"
#include "../AssemblerUnit/Parser/Statement.h"

#include <gmock/gmock.h>

#include <memory>
#include <vector>

using std::make_unique;
using std::move;
using std::unique_ptr;
using std::vector;

namespace {
    vector<unique_ptr<Statement>> assemble(const string& source) {
        AssemblerUnit assembler;
        return assembler.assemble(source);
    }

    vector<unique_ptr<Statement>> single(unique_ptr<Statement> statement) {
        vector<unique_ptr<Statement>> statements;
        statements.push_back(move(statement));

        return statements;
    }

    const BlockStmt* asBlock(const Statement* stmt) {
        return dynamic_cast<const BlockStmt*>(stmt);
    }

    const PrintStmt* asPrint(const Statement* stmt) {
        return dynamic_cast<const PrintStmt*>(stmt);
    }
}

class ResolverTestFixture : public testing::Test {
public:
    Resolver resolver;
};

TEST_F(ResolverTestFixture, VariableReferencedInItsOwnBlockResolvesToDistanceZero) {
    // { var a = 1; print a; }
    vector<unique_ptr<Statement>> statements = assemble("{ var a = 1; print a; }");
    const BlockStmt* block = asBlock(statements[0].get());
    const PrintStmt* print_stmt = asPrint(block->getStatements()[1].get());

    resolver.resolve(statements);

    const int* distance = resolver.getDistance(print_stmt->getExpr());
    ASSERT_NE(distance, nullptr);
    EXPECT_EQ(*distance, 0);
}

TEST_F(ResolverTestFixture, VariableReferencedFromNestedBlockResolvesToDistanceMatchingNesting) {
    // { var a = 1; { print a; } }
    vector<unique_ptr<Statement>> statements = assemble("{ var a = 1; { print a; } }");
    const BlockStmt* outer_block = asBlock(statements[0].get());
    const BlockStmt* inner_block = asBlock(outer_block->getStatements()[1].get());
    const PrintStmt* print_stmt = asPrint(inner_block->getStatements()[0].get());

    resolver.resolve(statements);

    const int* distance = resolver.getDistance(print_stmt->getExpr());
    ASSERT_NE(distance, nullptr);
    EXPECT_EQ(*distance, 1);
}

TEST_F(ResolverTestFixture, GlobalVariableReferencedInsideBlockHasNoDistance) {
    // var a = 1; { print a; }
    vector<unique_ptr<Statement>> statements = assemble("var a = 1; { print a; }");
    const BlockStmt* block = asBlock(statements[1].get());
    const PrintStmt* print_stmt = asPrint(block->getStatements()[0].get());

    resolver.resolve(statements);

    EXPECT_EQ(resolver.getDistance(print_stmt->getExpr()), nullptr);
}

TEST_F(ResolverTestFixture, AssignmentToVariableDeclaredInEnclosingBlockResolvesToMatchingDistance) {
    // { var a = 1; { a = 2; } }
    vector<unique_ptr<Statement>> statements = assemble("{ var a = 1; { a = 2; } }");
    const BlockStmt* outer_block = asBlock(statements[0].get());
    const BlockStmt* inner_block = asBlock(outer_block->getStatements()[1].get());
    const ExpressionStmt* assign_stmt = dynamic_cast<const ExpressionStmt*>(inner_block->getStatements()[0].get());

    resolver.resolve(statements);

    const int* distance = resolver.getDistance(assign_stmt->getExpr());
    ASSERT_NE(distance, nullptr);
    EXPECT_EQ(*distance, 1);
}

TEST_F(ResolverTestFixture, FunctionParameterReferencedInBodyResolvesToDistanceZero) {
    // Func f(x) { print x; }
    vector<unique_ptr<Statement>> statements = assemble("Func f(x) { print x; }");
    const FunctionStmt* function_stmt = dynamic_cast<const FunctionStmt*>(statements[0].get());
    const PrintStmt* print_stmt = asPrint(function_stmt->getBody()->getStatements()[0].get());

    resolver.resolve(statements);

    const int* distance = resolver.getDistance(print_stmt->getExpr());
    ASSERT_NE(distance, nullptr);
    EXPECT_EQ(*distance, 0);
}

TEST_F(ResolverTestFixture, ThisInsideMethodWithoutSuperclassResolvesToDistanceOne) {
    // Class Robot { move() { print this; } }
    vector<unique_ptr<Statement>> statements = assemble("Class Robot { move() { print this; } }");
    const ClassStmt* class_stmt = dynamic_cast<const ClassStmt*>(statements[0].get());
    const FunctionStmt* method = class_stmt->getMethods()[0].get();
    const PrintStmt* print_stmt = asPrint(method->getBody()->getStatements()[0].get());

    resolver.resolve(statements);

    const int* distance = resolver.getDistance(print_stmt->getExpr());
    ASSERT_NE(distance, nullptr);
    EXPECT_EQ(*distance, 1);
}

TEST_F(ResolverTestFixture, SuperInsideOverridingMethodResolvesToDistanceTwo) {
    // Class A { m() {} } Class B : A { m() { Super.m(); } }
    vector<unique_ptr<Statement>> statements = assemble("Class A { m() {} } Class B : A { m() { Super.m(); } }");
    const ClassStmt* subclass_stmt = dynamic_cast<const ClassStmt*>(statements[1].get());
    const FunctionStmt* method = subclass_stmt->getMethods()[0].get();
    const ExpressionStmt* super_call_stmt = dynamic_cast<const ExpressionStmt*>(method->getBody()->getStatements()[0].get());
    const CallExpr* call_expr = dynamic_cast<const CallExpr*>(super_call_stmt->getExpr());

    resolver.resolve(statements);

    const int* distance = resolver.getDistance(call_expr->getCallee());
    ASSERT_NE(distance, nullptr);
    EXPECT_EQ(*distance, 2);
}
