#include "../Statement.h"
#include "../Expression.h"
#include "../Parser.h"
#include "../../Tokenizer/Token.h"

#include <gmock/gmock.h>
#include <vector>

using std::vector;

TEST(ParserTest, VarDecalreStmt_SingleNumber_Passed) {
	vector<Token> tokens = {
		{TokenType::VAR, "var", "var", 1},
		{TokenType::IDENTIFIER, "a", "a", 1},
		{TokenType::EQUAL, "=", "=", 1},
		{TokenType::NUMBER, "10", 10.0, 1},
		{TokenType::SEMICOLON, ";", ";", 1},
		{TokenType::END_OF_FILE, "\n", "\n", 1}
	};

	Parser parser;
	VarDeclareStmt* stmt = dynamic_cast<VarDeclareStmt*>(parser.parse(tokens));

	ASSERT_NE(stmt, nullptr);

	const Token& name = stmt->getName();

	EXPECT_EQ(name.getType(), TokenType::IDENTIFIER);
	EXPECT_EQ(name.getLexeme(), "a");

	const LiteralExpr* expr = dynamic_cast<const LiteralExpr*>(stmt->getInitializer());

	ASSERT_NE(expr, nullptr);

	const Token& initializer = expr->getToken();

	EXPECT_EQ(initializer.getType(), TokenType::NUMBER);
	EXPECT_EQ(initializer.getLexeme(), "10");
	//EXPECT_EQ(stringify(initializer.getLiteral()), 10.0); // to avoid build error
	EXPECT_EQ(initializer.getLine(), 1);
}

TEST(ParserTest, VarDecalreStmt_SingleString_Passed) {
	vector<Token> tokens = {
	{TokenType::VAR, "var", "var", 1},
	{TokenType::IDENTIFIER, "a", "a", 1},
	{TokenType::EQUAL, "=", "=", 1},
	{TokenType::STRING, "\"text\"", "text", 1},
	{TokenType::SEMICOLON, ";", ";", 1},
	{TokenType::END_OF_FILE, "\n", "\n", 1}
	};

	Parser parser;
	VarDeclareStmt* stmt = dynamic_cast<VarDeclareStmt*>(parser.parse(tokens));

	ASSERT_NE(stmt, nullptr);

	const Token& name = stmt->getName();

	EXPECT_EQ(name.getType(), TokenType::IDENTIFIER);
	EXPECT_EQ(name.getLexeme(), "a");

	const LiteralExpr* expr = dynamic_cast<const LiteralExpr*>(stmt->getInitializer());

	ASSERT_NE(expr, nullptr);

	const Token& initializer = expr->getToken();

	EXPECT_EQ(initializer.getType(), TokenType::STRING);
	EXPECT_EQ(initializer.getLexeme(), "\"text\"");
	EXPECT_EQ(initializer.getLine(), 1);
}

TEST(ParserTest, VarDecalreStmt_SingleTrue_Passed) {
	vector<Token> tokens = {
		{TokenType::VAR, "var", "var", 1},
		{TokenType::IDENTIFIER, "a", "a", 1},
		{TokenType::EQUAL, "=", "=", 1},
		{TokenType::TRUE, "true", true, 1},
		{TokenType::SEMICOLON, ";", ";", 1},
		{TokenType::END_OF_FILE, "\n", "\n", 1}
	};

	Parser parser;
	VarDeclareStmt* stmt = dynamic_cast<VarDeclareStmt*>(parser.parse(tokens));

	ASSERT_NE(stmt, nullptr);

	const Token& name = stmt->getName();

	EXPECT_EQ(name.getType(), TokenType::IDENTIFIER);
	EXPECT_EQ(name.getLexeme(), "a");

	const LiteralExpr* expr = dynamic_cast<const LiteralExpr*>(stmt->getInitializer());

	ASSERT_NE(expr, nullptr);

	const Token& initializer = expr->getToken();

	EXPECT_EQ(initializer.getType(), TokenType::TRUE);
	EXPECT_EQ(initializer.getLexeme(), "true");
	EXPECT_EQ(initializer.getLine(), 1);
}

TEST(ParserTest, VarDecalreStmt_SingleFalse_Passed) {
	vector<Token> tokens = {
		{TokenType::VAR, "var", "var", 1},
		{TokenType::IDENTIFIER, "a", "a", 1},
		{TokenType::EQUAL, "=", "=", 1},
		{TokenType::FALSE, "false", false, 1},
		{TokenType::SEMICOLON, ";", ";", 1},
		{TokenType::END_OF_FILE, "\n", "\n", 1}
	};

	Parser parser;
	VarDeclareStmt* stmt = dynamic_cast<VarDeclareStmt*>(parser.parse(tokens));

	ASSERT_NE(stmt, nullptr);

	const Token& name = stmt->getName();

	EXPECT_EQ(name.getType(), TokenType::IDENTIFIER);
	EXPECT_EQ(name.getLexeme(), "a");

	const LiteralExpr* expr = dynamic_cast<const LiteralExpr*>(stmt->getInitializer());

	ASSERT_NE(expr, nullptr);

	const Token& initializer = expr->getToken();

	EXPECT_EQ(initializer.getType(), TokenType::FALSE);
	EXPECT_EQ(initializer.getLexeme(), "false");
	EXPECT_EQ(initializer.getLine(), 1);
}

TEST(ParserTest, VarDecalreStmt_NegativeNumber_Passed) {
	vector<Token> tokens = {
		{TokenType::VAR, "var", "var", 1},
		{TokenType::IDENTIFIER, "a", "a", 1},
		{TokenType::EQUAL, "=", "=", 1},
		{TokenType::MINUS, "-", "-", 1},
		{TokenType::NUMBER, "10", 10.0, 1},
		{TokenType::SEMICOLON, ";", ";", 1},
		{TokenType::END_OF_FILE, "\n", "\n", 1}
	};

	Parser parser;
	VarDeclareStmt* stmt = dynamic_cast<VarDeclareStmt*>(parser.parse(tokens));

	ASSERT_NE(stmt, nullptr);

	const Token& name = stmt->getName();

	EXPECT_EQ(name.getType(), TokenType::IDENTIFIER);
	EXPECT_EQ(name.getLexeme(), "a");

	const UnaryExpr* expr = dynamic_cast<const UnaryExpr*>(stmt->getInitializer());

	ASSERT_NE(expr, nullptr);

	const Token& op = expr->getOperator();

	EXPECT_EQ(op.getType(), TokenType::MINUS);
	EXPECT_EQ(op.getLexeme(), "-");

	const LiteralExpr* operand = dynamic_cast<const LiteralExpr*>(expr->getExpr());

	ASSERT_NE(operand, nullptr);

	const Token& operandToken = operand->getToken();

	EXPECT_EQ(operandToken.getType(), TokenType::NUMBER);
	EXPECT_EQ(operandToken.getLexeme(), "10");
}

TEST(ParserTest, VarDecalreStmt_NegatedTrue_Passed) {
	vector<Token> tokens = {
		{TokenType::VAR, "var", "var", 1},
		{TokenType::IDENTIFIER, "a", "a", 1},
		{TokenType::EQUAL, "=", "=", 1},
		{TokenType::BANG, "!", "!", 1},
		{TokenType::TRUE, "true", true, 1},
		{TokenType::SEMICOLON, ";", ";", 1},
		{TokenType::END_OF_FILE, "\n", "\n", 1}
	};

	Parser parser;
	VarDeclareStmt* stmt = dynamic_cast<VarDeclareStmt*>(parser.parse(tokens));

	ASSERT_NE(stmt, nullptr);

	const Token& name = stmt->getName();

	EXPECT_EQ(name.getType(), TokenType::IDENTIFIER);
	EXPECT_EQ(name.getLexeme(), "a");

	const UnaryExpr* expr = dynamic_cast<const UnaryExpr*>(stmt->getInitializer());

	ASSERT_NE(expr, nullptr);

	const Token& op = expr->getOperator();

	EXPECT_EQ(op.getType(), TokenType::BANG);
	EXPECT_EQ(op.getLexeme(), "!");

	const LiteralExpr* operand = dynamic_cast<const LiteralExpr*>(expr->getExpr());

	ASSERT_NE(operand, nullptr);

	const Token& operandToken = operand->getToken();

	EXPECT_EQ(operandToken.getType(), TokenType::TRUE);
	EXPECT_EQ(operandToken.getLexeme(), "true");
}

TEST(ParserTest, VarDecalreStmt_NegatedFalse_Passed) {
	vector<Token> tokens = {
		{TokenType::VAR, "var", "var", 1},
		{TokenType::IDENTIFIER, "a", "a", 1},
		{TokenType::EQUAL, "=", "=", 1},
		{TokenType::BANG, "!", "!", 1},
		{TokenType::FALSE, "false", false, 1},
		{TokenType::SEMICOLON, ";", ";", 1},
		{TokenType::END_OF_FILE, "\n", "\n", 1}
	};

	Parser parser;
	VarDeclareStmt* stmt = dynamic_cast<VarDeclareStmt*>(parser.parse(tokens));

	ASSERT_NE(stmt, nullptr);

	const Token& name = stmt->getName();

	EXPECT_EQ(name.getType(), TokenType::IDENTIFIER);
	EXPECT_EQ(name.getLexeme(), "a");

	const UnaryExpr* expr = dynamic_cast<const UnaryExpr*>(stmt->getInitializer());

	ASSERT_NE(expr, nullptr);

	const Token& op = expr->getOperator();

	EXPECT_EQ(op.getType(), TokenType::BANG);
	EXPECT_EQ(op.getLexeme(), "!");

	const LiteralExpr* operand = dynamic_cast<const LiteralExpr*>(expr->getExpr());

	ASSERT_NE(operand, nullptr);

	const Token& operandToken = operand->getToken();

	EXPECT_EQ(operandToken.getType(), TokenType::FALSE);
	EXPECT_EQ(operandToken.getLexeme(), "false");
}