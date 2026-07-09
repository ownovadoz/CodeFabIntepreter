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