#include "Interpreter.h"

#include "../AssemblerUnit/Parser/Expression.h"
#include "../AssemblerUnit/Parser/Statement.h"
#include "../AssemblerUnit/Tokenizer/Token.h"
#include "../CodeFabException.h"

#include <gmock/gmock.h>

#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <variant>

using std::make_unique;
using std::monostate;
using std::move;
using std::ostringstream;
using std::string;
using std::get;
using std::holds_alternative;

class InterpreterTestFixture : public testing::Test {
public:
    Interpreter interpreter;
};

class UnsupportedExpr : public Expression {
public:
    void accept(ExprVisitor&) const override {}
};

TEST_F(InterpreterTestFixture, VarDeclareWithInitializerDefinesVariable)
{
    Token name_token(TokenType::IDENTIFIER, "a", monostate{}, 1);
    Token literal_token(TokenType::NUMBER, "3", 3.0, 1);

    VarDeclareStmt var_decl(name_token);
    var_decl.setExpression(make_unique<LiteralExpr>(literal_token));

    interpreter.interpret(&var_decl);

    EXPECT_EQ(get<double>(interpreter.getVariableValue("a")), 3.0);
}

TEST_F(InterpreterTestFixture, VarDeclareWithoutInitializerDefinesNil)
{
    Token name_token(TokenType::IDENTIFIER, "a", monostate{}, 1);
    VarDeclareStmt var_decl(name_token);

    interpreter.interpret(&var_decl);

    EXPECT_TRUE(holds_alternative<monostate>(interpreter.getVariableValue("a")));
}

TEST_F(InterpreterTestFixture, BlockScopedVariableIsNotVisibleOutsideBlock)
{
    Token name_token(TokenType::IDENTIFIER, "a", monostate{}, 1);
    Token literal_token(TokenType::NUMBER, "3", 3.0, 1);

    auto var_decl = make_unique<VarDeclareStmt>(name_token);
    var_decl->setExpression(make_unique<LiteralExpr>(literal_token));

    BlockStmt block;
    block.addStatement(move(var_decl));

    interpreter.interpret(&block);

    EXPECT_THROW(interpreter.getVariableValue("a"), CodeFabException);
}

TEST_F(InterpreterTestFixture, EvaluateLiteralExprReturnsItsValue)
{
    Token literal_token(TokenType::STRING, "hi", string("hi"), 1);
    LiteralExpr literal(literal_token);

    Value value = interpreter.evaluate(&literal);

    EXPECT_EQ(get<string>(value), "hi");
}

TEST_F(InterpreterTestFixture, EvaluatingUnsupportedExpressionThrowsCodeFabException)
{
    UnsupportedExpr unsupported;

    EXPECT_THROW(interpreter.evaluate(&unsupported), CodeFabException);
}

TEST_F(InterpreterTestFixture, ExpressionStmtEvaluatesInnerExpressionWithoutThrowing)
{
    ExpressionStmt stmt(make_unique<LiteralExpr>(Token(TokenType::NUMBER, "5", 5.0, 1)));

    EXPECT_NO_THROW(interpreter.interpret(&stmt));
}

TEST_F(InterpreterTestFixture, ExpressionStmtPropagatesEvaluationErrors)
{
    ExpressionStmt stmt(make_unique<UnsupportedExpr>());

    EXPECT_THROW(interpreter.interpret(&stmt), CodeFabException);
}

TEST_F(InterpreterTestFixture, PrintStmtWritesStringifiedValueToStdout)
{
    PrintStmt stmt(make_unique<LiteralExpr>(Token(TokenType::NUMBER, "5", 5.0, 1)));

    ostringstream captured;
    std::streambuf* original_buf = std::cout.rdbuf(captured.rdbuf());
    interpreter.interpret(&stmt);
    std::cout.rdbuf(original_buf);

    EXPECT_EQ(captured.str(), "5\n");
}

TEST_F(InterpreterTestFixture, IfStmtExecutesThenBranchWhenConditionIsTruthy)
{
    auto then_branch = make_unique<VarDeclareStmt>(Token(TokenType::IDENTIFIER, "a", monostate{}, 1));
    then_branch->setExpression(make_unique<LiteralExpr>(Token(TokenType::NUMBER, "1", 1.0, 1)));
    auto else_branch = make_unique<VarDeclareStmt>(Token(TokenType::IDENTIFIER, "a", monostate{}, 1));
    else_branch->setExpression(make_unique<LiteralExpr>(Token(TokenType::NUMBER, "2", 2.0, 1)));

    IfStmt if_stmt(make_unique<LiteralExpr>(Token(TokenType::TRUE, "true", true, 1)), move(then_branch), move(else_branch));
    interpreter.interpret(&if_stmt);

    EXPECT_EQ(get<double>(interpreter.getVariableValue("a")), 1.0);
}

TEST_F(InterpreterTestFixture, IfStmtExecutesElseBranchWhenConditionIsFalsy)
{
    auto then_branch = make_unique<VarDeclareStmt>(Token(TokenType::IDENTIFIER, "a", monostate{}, 1));
    then_branch->setExpression(make_unique<LiteralExpr>(Token(TokenType::NUMBER, "1", 1.0, 1)));
    auto else_branch = make_unique<VarDeclareStmt>(Token(TokenType::IDENTIFIER, "a", monostate{}, 1));
    else_branch->setExpression(make_unique<LiteralExpr>(Token(TokenType::NUMBER, "2", 2.0, 1)));

    IfStmt if_stmt(make_unique<LiteralExpr>(Token(TokenType::FALSE, "false", false, 1)), move(then_branch), move(else_branch));
    interpreter.interpret(&if_stmt);

    EXPECT_EQ(get<double>(interpreter.getVariableValue("a")), 2.0);
}

TEST_F(InterpreterTestFixture, IfStmtDoesNothingWhenConditionIsFalsyAndNoElseBranch)
{
    auto then_branch = make_unique<VarDeclareStmt>(Token(TokenType::IDENTIFIER, "a", monostate{}, 1));
    then_branch->setExpression(make_unique<LiteralExpr>(Token(TokenType::NUMBER, "1", 1.0, 1)));

    IfStmt if_stmt(make_unique<LiteralExpr>(Token(TokenType::FALSE, "false", false, 1)), move(then_branch), nullptr);
    interpreter.interpret(&if_stmt);

    EXPECT_THROW(interpreter.getVariableValue("a"), CodeFabException);
}

TEST_F(InterpreterTestFixture, EvaluateVariableExprReturnsItsCurrentValue)
{
    Token name_token(TokenType::IDENTIFIER, "a", monostate{}, 1);
    Token literal_token(TokenType::NUMBER, "3", 3.0, 1);

    VarDeclareStmt var_decl(name_token);
    var_decl.setExpression(make_unique<LiteralExpr>(literal_token));
    interpreter.interpret(&var_decl);

    VariableExpr variable(name_token);

    EXPECT_EQ(get<double>(interpreter.evaluate(&variable)), 3.0);
}

TEST_F(InterpreterTestFixture, EvaluatingAssignExprUpdatesEnvironmentAndReturnsValue)
{
    Token name_token(TokenType::IDENTIFIER, "a", monostate{}, 1);
    VarDeclareStmt var_decl(name_token);
    var_decl.setExpression(make_unique<LiteralExpr>(Token(TokenType::NUMBER, "1", 1.0, 1)));
    interpreter.interpret(&var_decl);

    AssignExpr assign(name_token, make_unique<LiteralExpr>(Token(TokenType::NUMBER, "2", 2.0, 1)));

    EXPECT_EQ(get<double>(interpreter.evaluate(&assign)), 2.0);
    EXPECT_EQ(get<double>(interpreter.getVariableValue("a")), 2.0);
}

TEST_F(InterpreterTestFixture, BlockScopedVariableShadowsOuterVariableAndDoesNotLeakOut)
{
    // var x = "global";
    // { var x = "inner"; print x; }
    // print x;
    auto outer_decl = make_unique<VarDeclareStmt>(Token(TokenType::IDENTIFIER, "x", monostate{}, 1));
    outer_decl->setExpression(make_unique<LiteralExpr>(Token(TokenType::STRING, "global", string("global"), 1)));

    auto inner_decl = make_unique<VarDeclareStmt>(Token(TokenType::IDENTIFIER, "x", monostate{}, 2));
    inner_decl->setExpression(make_unique<LiteralExpr>(Token(TokenType::STRING, "inner", string("inner"), 2)));
    auto inner_print = make_unique<PrintStmt>(make_unique<VariableExpr>(Token(TokenType::IDENTIFIER, "x", monostate{}, 2)));

    auto block = make_unique<BlockStmt>();
    block->addStatement(move(inner_decl));
    block->addStatement(move(inner_print));

    auto outer_print = make_unique<PrintStmt>(make_unique<VariableExpr>(Token(TokenType::IDENTIFIER, "x", monostate{}, 4)));

    ostringstream captured;
    std::streambuf* original_buf = std::cout.rdbuf(captured.rdbuf());

    interpreter.interpret(outer_decl.get());
    interpreter.interpret(block.get());
    interpreter.interpret(outer_print.get());

    std::cout.rdbuf(original_buf);

    EXPECT_EQ(captured.str(), "inner\nglobal\n");
}

TEST_F(InterpreterTestFixture, AcceptOnVarDeclareStmtDispatchesToInterpreter)
{
    Token name_token(TokenType::IDENTIFIER, "a", monostate{}, 1);
    VarDeclareStmt var_decl(name_token);
    var_decl.setExpression(make_unique<LiteralExpr>(Token(TokenType::NUMBER, "7", 7.0, 1)));

    var_decl.accept(interpreter);

    EXPECT_EQ(get<double>(interpreter.getVariableValue("a")), 7.0);
}

TEST_F(InterpreterTestFixture, AcceptOnLiteralExprDoesNotThrow)
{
    LiteralExpr literal(Token(TokenType::NUMBER, "9", 9.0, 1));

    EXPECT_NO_THROW(literal.accept(interpreter));
}

TEST_F(InterpreterTestFixture, ForStmtDoesNotExecuteBodyWhenConditionIsInitiallyFalsy)
{
    auto init = make_unique<VarDeclareStmt>(Token(TokenType::IDENTIFIER, "i", monostate{}, 1));
    init->setExpression(make_unique<LiteralExpr>(Token(TokenType::NUMBER, "0", 0.0, 1)));
    auto condition = make_unique<LiteralExpr>(Token(TokenType::FALSE, "false", false, 1));
    auto body = make_unique<VarDeclareStmt>(Token(TokenType::IDENTIFIER, "a", monostate{}, 1));
    body->setExpression(make_unique<LiteralExpr>(Token(TokenType::NUMBER, "9", 9.0, 1)));

    ForStmt for_stmt(move(init), move(condition), nullptr, move(body));
    interpreter.interpret(&for_stmt);

    EXPECT_THROW(interpreter.getVariableValue("a"), CodeFabException);
    EXPECT_THROW(interpreter.getVariableValue("i"), CodeFabException);
}

TEST_F(InterpreterTestFixture, BinaryPlusAddsNumbers)
{
    Token plus_token(TokenType::PLUS, "+", monostate{}, 1);
    BinaryExpr binary(
        make_unique<LiteralExpr>(Token(TokenType::NUMBER, "2", 2.0, 1)),
        plus_token,
        make_unique<LiteralExpr>(Token(TokenType::NUMBER, "3", 3.0, 1)));

    EXPECT_EQ(get<double>(interpreter.evaluate(&binary)), 5.0);
}

TEST_F(InterpreterTestFixture, BinaryPlusConcatenatesStrings)
{
    Token plus_token(TokenType::PLUS, "+", monostate{}, 1);
    BinaryExpr binary(
        make_unique<LiteralExpr>(Token(TokenType::STRING, "a", string("a"), 1)),
        plus_token,
        make_unique<LiteralExpr>(Token(TokenType::STRING, "b", string("b"), 1)));

    EXPECT_EQ(get<string>(interpreter.evaluate(&binary)), "ab");
}

TEST_F(InterpreterTestFixture, BinaryPlusOnMismatchedTypesThrowsCodeFabException)
{
    Token plus_token(TokenType::PLUS, "+", monostate{}, 1);
    BinaryExpr binary(
        make_unique<LiteralExpr>(Token(TokenType::NUMBER, "2", 2.0, 1)),
        plus_token,
        make_unique<LiteralExpr>(Token(TokenType::STRING, "b", string("b"), 1)));

    EXPECT_THROW(interpreter.evaluate(&binary), CodeFabException);
}

TEST_F(InterpreterTestFixture, BinaryMinusOnNonNumberThrowsCodeFabException)
{
    Token minus_token(TokenType::MINUS, "-", monostate{}, 1);
    BinaryExpr binary(
        make_unique<LiteralExpr>(Token(TokenType::STRING, "a", string("a"), 1)),
        minus_token,
        make_unique<LiteralExpr>(Token(TokenType::NUMBER, "1", 1.0, 1)));

    EXPECT_THROW(interpreter.evaluate(&binary), CodeFabException);
}

TEST_F(InterpreterTestFixture, BinarySlashByZeroThrowsCodeFabException)
{
    Token slash_token(TokenType::SLASH, "/", monostate{}, 1);
    BinaryExpr binary(
        make_unique<LiteralExpr>(Token(TokenType::NUMBER, "3", 3.0, 1)),
        slash_token,
        make_unique<LiteralExpr>(Token(TokenType::NUMBER, "0", 0.0, 1)));

    EXPECT_THROW(interpreter.evaluate(&binary), CodeFabException);
}

TEST_F(InterpreterTestFixture, BinaryGreaterComparesNumbers)
{
    Token greater_token(TokenType::GREATER, ">", monostate{}, 1);
    BinaryExpr binary(
        make_unique<LiteralExpr>(Token(TokenType::NUMBER, "5", 5.0, 1)),
        greater_token,
        make_unique<LiteralExpr>(Token(TokenType::NUMBER, "3", 3.0, 1)));

    EXPECT_TRUE(get<bool>(interpreter.evaluate(&binary)));
}

TEST_F(InterpreterTestFixture, BinaryGreaterEqualComparesNumbers)
{
    Token op_token(TokenType::GREATER_EQUAL, ">=", monostate{}, 1);
    BinaryExpr binary(
        make_unique<LiteralExpr>(Token(TokenType::NUMBER, "3", 3.0, 1)),
        op_token,
        make_unique<LiteralExpr>(Token(TokenType::NUMBER, "3", 3.0, 1)));

    EXPECT_TRUE(get<bool>(interpreter.evaluate(&binary)));
}

TEST_F(InterpreterTestFixture, BinaryEqualEqualComparesValuesOfSameType)
{
    Token op_token(TokenType::EQUAL_EQUAL, "==", monostate{}, 1);
    BinaryExpr binary(
        make_unique<LiteralExpr>(Token(TokenType::NUMBER, "3", 3.0, 1)),
        op_token,
        make_unique<LiteralExpr>(Token(TokenType::NUMBER, "3", 3.0, 1)));

    EXPECT_TRUE(get<bool>(interpreter.evaluate(&binary)));
}

TEST_F(InterpreterTestFixture, BinaryBangEqualReturnsTrueForDifferentValues)
{
    Token op_token(TokenType::BANG_EQUAL, "!=", monostate{}, 1);
    BinaryExpr binary(
        make_unique<LiteralExpr>(Token(TokenType::NUMBER, "3", 3.0, 1)),
        op_token,
        make_unique<LiteralExpr>(Token(TokenType::STRING, "3", string("3"), 1)));

    EXPECT_TRUE(get<bool>(interpreter.evaluate(&binary)));
}

TEST_F(InterpreterTestFixture, EvaluatingUnaryMinusNegatesNumber)
{
    Token op_token(TokenType::MINUS, "-", monostate{}, 1);
    UnaryExpr unary(op_token, make_unique<LiteralExpr>(Token(TokenType::NUMBER, "3", 3.0, 1)));

    EXPECT_EQ(get<double>(interpreter.evaluate(&unary)), -3.0);
}

TEST_F(InterpreterTestFixture, EvaluatingUnaryMinusOnNonNumberThrowsCodeFabException)
{
    Token op_token(TokenType::MINUS, "-", monostate{}, 1);
    UnaryExpr unary(op_token, make_unique<LiteralExpr>(Token(TokenType::STRING, "hi", string("hi"), 1)));

    EXPECT_THROW(interpreter.evaluate(&unary), CodeFabException);
}

TEST_F(InterpreterTestFixture, EvaluatingUnaryBangNegatesTruthiness)
{
    Token op_token(TokenType::BANG, "!", monostate{}, 1);
    UnaryExpr unary(op_token, make_unique<LiteralExpr>(Token(TokenType::FALSE, "false", false, 1)));

    EXPECT_TRUE(get<bool>(interpreter.evaluate(&unary)));
}

TEST_F(InterpreterTestFixture, EvaluatingGroupingExprReturnsInnerValue)
{
    GroupingExpr grouping(make_unique<LiteralExpr>(Token(TokenType::NUMBER, "5", 5.0, 1)));

    EXPECT_EQ(get<double>(interpreter.evaluate(&grouping)), 5.0);
}

TEST_F(InterpreterTestFixture, EvaluatingGroupingExprPropagatesInnerErrors)
{
    Token op_token(TokenType::MINUS, "-", monostate{}, 1);
    GroupingExpr grouping(make_unique<UnaryExpr>(op_token, make_unique<LiteralExpr>(Token(TokenType::STRING, "hi", string("hi"), 1))));

    EXPECT_THROW(interpreter.evaluate(&grouping), CodeFabException);
}

TEST_F(InterpreterTestFixture, LogicalAndShortCircuitsOnFalsyLeftWithoutEvaluatingRight)
{
    Token and_token(TokenType::AND, "and", monostate{}, 1);
    LogicalExpr logical(
        make_unique<LiteralExpr>(Token(TokenType::FALSE, "false", false, 1)),
        and_token,
        make_unique<VariableExpr>(Token(TokenType::IDENTIFIER, "undefined", monostate{}, 1)));

    EXPECT_FALSE(get<bool>(interpreter.evaluate(&logical)));
}

TEST_F(InterpreterTestFixture, LogicalOrShortCircuitsOnTruthyLeftWithoutEvaluatingRight)
{
    Token or_token(TokenType::OR, "or", monostate{}, 1);
    LogicalExpr logical(
        make_unique<LiteralExpr>(Token(TokenType::TRUE, "true", true, 1)),
        or_token,
        make_unique<VariableExpr>(Token(TokenType::IDENTIFIER, "undefined", monostate{}, 1)));

    EXPECT_TRUE(get<bool>(interpreter.evaluate(&logical)));
}

TEST_F(InterpreterTestFixture, LogicalAndEvaluatesRightWhenLeftIsTruthy)
{
    Token and_token(TokenType::AND, "and", monostate{}, 1);
    LogicalExpr logical(
        make_unique<LiteralExpr>(Token(TokenType::TRUE, "true", true, 1)),
        and_token,
        make_unique<LiteralExpr>(Token(TokenType::NUMBER, "5", 5.0, 1)));

    EXPECT_EQ(get<double>(interpreter.evaluate(&logical)), 5.0);
}

TEST_F(InterpreterTestFixture, LogicalOrEvaluatesRightWhenLeftIsFalsy)
{
    Token or_token(TokenType::OR, "or", monostate{}, 1);
    LogicalExpr logical(
        make_unique<LiteralExpr>(Token(TokenType::FALSE, "false", false, 1)),
        or_token,
        make_unique<LiteralExpr>(Token(TokenType::NUMBER, "5", 5.0, 1)));

    EXPECT_EQ(get<double>(interpreter.evaluate(&logical)), 5.0);
}