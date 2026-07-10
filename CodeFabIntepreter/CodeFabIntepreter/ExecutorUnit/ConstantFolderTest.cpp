#include "ConstantFolder.h"

#include "Interpreter.h"

#include "../AssemblerUnit/Parser/Expression.h"
#include "../AssemblerUnit/Parser/Statement.h"
#include "../AssemblerUnit/Tokenizer/Token.h"

#include <gmock/gmock.h>

#include <memory>
#include <variant>
#include <vector>

using std::get;
using std::make_unique;
using std::monostate;
using std::move;
using std::unique_ptr;
using std::vector;

namespace {
    vector<unique_ptr<Statement>> single(unique_ptr<Statement> statement) {
        vector<unique_ptr<Statement>> statements;
        statements.push_back(move(statement));

        return statements;
    }

    unique_ptr<LiteralExpr> numberLiteral(double value) {
        return make_unique<LiteralExpr>(Token(TokenType::NUMBER, std::to_string(value), value, 1));
    }

    unique_ptr<LiteralExpr> boolLiteral(bool value) {
        return make_unique<LiteralExpr>(Token(value ? TokenType::TRUE : TokenType::FALSE, value ? "true" : "false", value, 1));
    }
}

class ConstantFolderTestFixture : public testing::Test {
public:
    Interpreter interpreter;
    ConstantFolder folder{ interpreter };
};

TEST_F(ConstantFolderTestFixture, FoldsBinaryExprWithLiteralOperands) {
    auto binary = make_unique<BinaryExpr>(numberLiteral(1.0), Token(TokenType::PLUS, "+", monostate{}, 1), numberLiteral(2.0));
    const BinaryExpr* binary_ptr = binary.get();

    folder.fold(single(make_unique<ExpressionStmt>(move(binary))));

    ASSERT_EQ(folder.getFoldedValues().count(binary_ptr), 1u);
    EXPECT_EQ(get<double>(folder.getFoldedValues().at(binary_ptr)), 3.0);
}

TEST_F(ConstantFolderTestFixture, DoesNotFoldBinaryExprWithVariableOperand) {
    auto binary = make_unique<BinaryExpr>(
        make_unique<VariableExpr>(Token(TokenType::IDENTIFIER, "a", monostate{}, 1)),
        Token(TokenType::PLUS, "+", monostate{}, 1),
        numberLiteral(1.0));
    const BinaryExpr* binary_ptr = binary.get();

    folder.fold(single(make_unique<ExpressionStmt>(move(binary))));

    EXPECT_EQ(folder.getFoldedValues().count(binary_ptr), 0u);
}

TEST_F(ConstantFolderTestFixture, FoldsNestedUnaryGroupingAndBinaryExpr) {
    // -(1 * 2)
    auto inner_binary = make_unique<BinaryExpr>(numberLiteral(1.0), Token(TokenType::STAR, "*", monostate{}, 1), numberLiteral(2.0));
    auto grouping = make_unique<GroupingExpr>(move(inner_binary));
    const GroupingExpr* grouping_ptr = grouping.get();
    auto unary = make_unique<UnaryExpr>(Token(TokenType::MINUS, "-", monostate{}, 1), move(grouping));
    const UnaryExpr* unary_ptr = unary.get();

    folder.fold(single(make_unique<ExpressionStmt>(move(unary))));

    ASSERT_EQ(folder.getFoldedValues().count(grouping_ptr), 1u);
    EXPECT_EQ(get<double>(folder.getFoldedValues().at(grouping_ptr)), 2.0);

    ASSERT_EQ(folder.getFoldedValues().count(unary_ptr), 1u);
    EXPECT_EQ(get<double>(folder.getFoldedValues().at(unary_ptr)), -2.0);
}

TEST_F(ConstantFolderTestFixture, DoesNotFoldExpressionThatWouldThrowAtRuntime) {
    // 1 / 0은 실행 시점에 예외를 던져야 하므로, 미리 계산해서 접어두면 안 된다.
    auto binary = make_unique<BinaryExpr>(numberLiteral(1.0), Token(TokenType::SLASH, "/", monostate{}, 1), numberLiteral(0.0));
    const BinaryExpr* binary_ptr = binary.get();

    folder.fold(single(make_unique<ExpressionStmt>(move(binary))));

    EXPECT_EQ(folder.getFoldedValues().count(binary_ptr), 0u);
}

TEST_F(ConstantFolderTestFixture, FoldsLogicalExprWithLiteralOperands) {
    auto logical = make_unique<LogicalExpr>(boolLiteral(true), Token(TokenType::AND, "and", monostate{}, 1), boolLiteral(false));
    const LogicalExpr* logical_ptr = logical.get();

    folder.fold(single(make_unique<ExpressionStmt>(move(logical))));

    ASSERT_EQ(folder.getFoldedValues().count(logical_ptr), 1u);
    EXPECT_EQ(get<bool>(folder.getFoldedValues().at(logical_ptr)), false);
}

TEST_F(ConstantFolderTestFixture, TraversesIntoNestedStatementsToFindFoldableExpressions) {
    auto binary = make_unique<BinaryExpr>(numberLiteral(4.0), Token(TokenType::PLUS, "+", monostate{}, 1), numberLiteral(5.0));
    const BinaryExpr* binary_ptr = binary.get();

    auto print_stmt = make_unique<PrintStmt>(move(binary));
    auto block = make_unique<BlockStmt>();
    block->addStatement(move(print_stmt));

    folder.fold(single(move(block)));

    ASSERT_EQ(folder.getFoldedValues().count(binary_ptr), 1u);
    EXPECT_EQ(get<double>(folder.getFoldedValues().at(binary_ptr)), 9.0);
}
