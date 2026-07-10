#include "LineResolver.h"

#include "../AssemblerUnit/Parser/Expression.h"
#include "../AssemblerUnit/Parser/Statement.h"
#include "../AssemblerUnit/Tokenizer/Token.h"

#include <gmock/gmock.h>

#include <memory>
#include <variant>
#include <vector>

using std::make_unique;
using std::monostate;
using std::move;
using std::unique_ptr;
using std::vector;

namespace {
    unique_ptr<LiteralExpr> numberLiteral(double value, int line) {
        return make_unique<LiteralExpr>(Token(TokenType::NUMBER, std::to_string(value), value, line));
    }
}

class LineResolverTestFixture : public testing::Test {
public:
    LineResolver line_resolver;
};

TEST_F(LineResolverTestFixture, BinaryExprResolvesToOperatorLine) {
    BinaryExpr expr(numberLiteral(1.0, 1), Token(TokenType::PLUS, "+", monostate{}, 7), numberLiteral(2.0, 1));

    EXPECT_EQ(line_resolver.resolve(&expr), 7);
}

TEST_F(LineResolverTestFixture, UnaryExprResolvesToOperatorLine) {
    UnaryExpr expr(Token(TokenType::MINUS, "-", monostate{}, 3), numberLiteral(1.0, 1));

    EXPECT_EQ(line_resolver.resolve(&expr), 3);
}

TEST_F(LineResolverTestFixture, GroupingExprResolvesToInnerExprLine) {
    GroupingExpr expr(numberLiteral(1.0, 5));

    EXPECT_EQ(line_resolver.resolve(&expr), 5);
}

TEST_F(LineResolverTestFixture, LogicalExprResolvesToOperatorLine) {
    LogicalExpr expr(numberLiteral(1.0, 1), Token(TokenType::AND, "and", monostate{}, 4), numberLiteral(2.0, 1));

    EXPECT_EQ(line_resolver.resolve(&expr), 4);
}

TEST_F(LineResolverTestFixture, CallExprResolvesToClosingParenLine) {
    vector<unique_ptr<Expression>> arguments;
    CallExpr expr(make_unique<VariableExpr>(Token(TokenType::IDENTIFIER, "f", monostate{}, 1)), Token(TokenType::RIGHT_PAREN, ")", monostate{}, 6), move(arguments));

    EXPECT_EQ(line_resolver.resolve(&expr), 6);
}

TEST_F(LineResolverTestFixture, GetExprResolvesToFieldNameLine) {
    GetExpr expr(make_unique<VariableExpr>(Token(TokenType::IDENTIFIER, "obj", monostate{}, 1)), Token(TokenType::IDENTIFIER, "field", monostate{}, 8));

    EXPECT_EQ(line_resolver.resolve(&expr), 8);
}

TEST_F(LineResolverTestFixture, SetExprResolvesToFieldNameLine) {
    SetExpr expr(
        make_unique<VariableExpr>(Token(TokenType::IDENTIFIER, "obj", monostate{}, 1)),
        Token(TokenType::IDENTIFIER, "field", monostate{}, 9),
        numberLiteral(1.0, 1));

    EXPECT_EQ(line_resolver.resolve(&expr), 9);
}

TEST_F(LineResolverTestFixture, ThisExprResolvesToKeywordLine) {
    ThisExpr expr(Token(TokenType::THIS, "this", monostate{}, 2));

    EXPECT_EQ(line_resolver.resolve(&expr), 2);
}

TEST_F(LineResolverTestFixture, SuperExprResolvesToKeywordLine) {
    SuperExpr expr(Token(TokenType::SUPER, "Super", monostate{}, 11), Token(TokenType::IDENTIFIER, "move", monostate{}, 11));

    EXPECT_EQ(line_resolver.resolve(&expr), 11);
}

TEST_F(LineResolverTestFixture, InstanceOfExprResolvesToKeywordLine) {
    InstanceOfExpr expr(
        make_unique<VariableExpr>(Token(TokenType::IDENTIFIER, "w", monostate{}, 1)),
        Token(TokenType::INSTANCEOF, "instanceof", monostate{}, 12),
        Token(TokenType::IDENTIFIER, "Robot", monostate{}, 12));

    EXPECT_EQ(line_resolver.resolve(&expr), 12);
}

TEST_F(LineResolverTestFixture, ArrayExprResolvesToSizeExprLine) {
    ArrayExpr expr(numberLiteral(3.0, 13));

    EXPECT_EQ(line_resolver.resolve(&expr), 13);
}

TEST_F(LineResolverTestFixture, IndexExprResolvesToArrayExprLine) {
    IndexExpr expr(make_unique<VariableExpr>(Token(TokenType::IDENTIFIER, "arr", monostate{}, 14)), numberLiteral(0.0, 14));

    EXPECT_EQ(line_resolver.resolve(&expr), 14);
}

TEST_F(LineResolverTestFixture, IndexSetExprResolvesToArrayExprLine) {
    IndexSetExpr expr(
        make_unique<VariableExpr>(Token(TokenType::IDENTIFIER, "arr", monostate{}, 15)),
        numberLiteral(0.0, 15),
        numberLiteral(1.0, 15));

    EXPECT_EQ(line_resolver.resolve(&expr), 15);
}

TEST_F(LineResolverTestFixture, IfStmtResolvesToConditionExprLine) {
    IfStmt stmt(numberLiteral(1.0, 16), nullptr, nullptr);

    EXPECT_EQ(line_resolver.resolve(&stmt), 16);
}

TEST_F(LineResolverTestFixture, BlockStmtResolvesToFirstStatementLine) {
    BlockStmt block;
    block.addStatement(make_unique<ExpressionStmt>(numberLiteral(1.0, 17)));

    EXPECT_EQ(line_resolver.resolve(&block), 17);
}

TEST_F(LineResolverTestFixture, EmptyBlockStmtResolvesToZero) {
    BlockStmt block;

    EXPECT_EQ(line_resolver.resolve(&block), 0);
}

TEST_F(LineResolverTestFixture, BlockStmtWithOnlyNestedEmptyBlockLoopsToCompletionAndResolvesToZero) {
    // 안쪽 BlockStmt도 비어있어 0을 반환하므로, 바깥 반복문이 "if (line != 0)
    // return;"으로 조기 종료하지 않고 끝까지 돈 뒤 자연스럽게 0을 반환하는
    // 경로를 검증한다.
    BlockStmt block;
    block.addStatement(make_unique<BlockStmt>());

    EXPECT_EQ(line_resolver.resolve(&block), 0);
}

TEST_F(LineResolverTestFixture, ForStmtWithInitResolvesToInitLine) {
    auto init = make_unique<VarDeclareStmt>(Token(TokenType::IDENTIFIER, "i", monostate{}, 18));
    init->setExpression(numberLiteral(0.0, 18));

    ForStmt stmt(move(init), nullptr, nullptr, nullptr);

    EXPECT_EQ(line_resolver.resolve(&stmt), 18);
}

TEST_F(LineResolverTestFixture, ForStmtWithoutInitButWithConditionResolvesToConditionLine) {
    ForStmt stmt(nullptr, numberLiteral(1.0, 19), nullptr, nullptr);

    EXPECT_EQ(line_resolver.resolve(&stmt), 19);
}

TEST_F(LineResolverTestFixture, ForStmtWithOnlyIncrementResolvesToIncrementLine) {
    ForStmt stmt(nullptr, nullptr, numberLiteral(1.0, 20), nullptr);

    EXPECT_EQ(line_resolver.resolve(&stmt), 20);
}

TEST_F(LineResolverTestFixture, ForStmtWithOnlyBodyResolvesToBodyLine) {
    ForStmt stmt(nullptr, nullptr, nullptr, make_unique<ExpressionStmt>(numberLiteral(1.0, 21)));

    EXPECT_EQ(line_resolver.resolve(&stmt), 21);
}

TEST_F(LineResolverTestFixture, ReturnStmtResolvesToKeywordLine) {
    ReturnStmt stmt(Token(TokenType::RETURN, "return", monostate{}, 22), nullptr);

    EXPECT_EQ(line_resolver.resolve(&stmt), 22);
}

TEST_F(LineResolverTestFixture, ClassStmtResolvesToNameLine) {
    ClassStmt stmt(Token(TokenType::IDENTIFIER, "Robot", monostate{}, 23), nullptr, vector<unique_ptr<FunctionStmt>>{});

    EXPECT_EQ(line_resolver.resolve(&stmt), 23);
}

TEST_F(LineResolverTestFixture, ImportStmtResolvesToPathLine) {
    ImportStmt stmt(Token(TokenType::STRING, "\"a.txt\"", string("a.txt"), 24), Token(TokenType::IDENTIFIER, "a", monostate{}, 24));

    EXPECT_EQ(line_resolver.resolve(&stmt), 24);
}
