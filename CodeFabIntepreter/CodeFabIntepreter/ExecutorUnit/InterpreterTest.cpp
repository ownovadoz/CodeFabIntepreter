#include "Interpreter.h"

#include "../AssemblerUnit/Parser/Expression.h"
#include "../AssemblerUnit/Parser/Statement.h"
#include "../AssemblerUnit/Tokenizer/Token.h"
#include "../RuntimeError.h"

#include <gmock/gmock.h>

#include <string>
#include <variant>

using std::monostate;
using std::string;
using std::get;
using std::holds_alternative;

class InterpreterTestFixture : public testing::Test {
public:
    Interpreter interpreter;
};

// Checker unit이 변수 중복 선언, 스코프 규칙을 이미 검증하므로 여기서는 실행 동작만 검증한다.

TEST_F(InterpreterTestFixture, VarDeclareWithInitializerDefinesVariable)
{
    Token name_token(TokenType::IDENTIFIER, "a", monostate{}, 1);
    Token literal_token(TokenType::NUMBER, "3", 3.0, 1);

    VarDeclareStmt var_decl(name_token);
    LiteralExpr initializer(literal_token);
    var_decl.setExpression(&initializer);

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

    VarDeclareStmt* var_decl = new VarDeclareStmt(name_token);
    LiteralExpr* initializer = new LiteralExpr(literal_token);
    var_decl->setExpression(initializer);

    BlockStmt block;
    block.addStatement(var_decl);

    interpreter.interpret(&block);

    EXPECT_THROW(interpreter.getVariableValue("a"), RuntimeError);

    delete initializer;
    delete var_decl;
}

TEST_F(InterpreterTestFixture, EvaluateLiteralExprReturnsItsValue)
{
    Token literal_token(TokenType::STRING, "hi", string("hi"), 1);
    LiteralExpr literal(literal_token);

    Value value = interpreter.evaluate(&literal);

    EXPECT_EQ(get<string>(value), "hi");
}

TEST_F(InterpreterTestFixture, EvaluatingUnsupportedExpressionThrowsRuntimeError)
{
    Expression unsupported;

    EXPECT_THROW(interpreter.evaluate(&unsupported), RuntimeError);
}