#include <gmock/gmock.h>

#include "Parser.h"
#include "Statement.h"
#include "Expression.h"

TEST(ParserTest, ParsesVarStatementWithLiteralInitializer) {
	// var a = 10;
	vector<Token> tokens = {
		Token(TokenType::VAR, "var"),
		Token(TokenType::IDENTIFIER, "a"),
		Token(TokenType::EQUAL, "="),
		Token(TokenType::NUMBER, "10"),
		Token(TokenType::SEMICOLON, ";"),
		Token(TokenType::END_OF_FILE, ""),
	};

	Parser parser(tokens);
	shared_ptr<Node> node = parser.parse();

	auto* stmt = dynamic_cast<VarDeclareStmt*>(node.get());
	ASSERT_NE(stmt, nullptr);
	ASSERT_NE(stmt->getName(), nullptr);
	EXPECT_EQ(stmt->getName()->getLexeme(), "a");

	auto literal = std::dynamic_pointer_cast<LiteralExpr>(stmt->getInitializer());
	ASSERT_NE(literal, nullptr);
	EXPECT_EQ(literal->getValue().getLexeme(), "10");
}

TEST(ParserTest, ParsesVarStatementWithBinaryInitializer) {
	// var a = 3 + 5;
	vector<Token> tokens = {
		Token(TokenType::VAR, "var"),
		Token(TokenType::IDENTIFIER, "a"),
		Token(TokenType::EQUAL, "="),
		Token(TokenType::NUMBER, "3"),
		Token(TokenType::PLUS, "+"),
		Token(TokenType::NUMBER, "5"),
		Token(TokenType::SEMICOLON, ";"),
		Token(TokenType::END_OF_FILE, ""),
	};

	Parser parser(tokens);
	shared_ptr<Node> node = parser.parse();

	auto* stmt = dynamic_cast<VarDeclareStmt*>(node.get());
	ASSERT_NE(stmt, nullptr);
	ASSERT_NE(stmt->getName(), nullptr);
	EXPECT_EQ(stmt->getName()->getLexeme(), "a");

	auto binary = std::dynamic_pointer_cast<BinaryExpr>(stmt->getInitializer());
	ASSERT_NE(binary, nullptr);
	EXPECT_EQ(binary->getOp().getType(), TokenType::PLUS);

	auto left = std::dynamic_pointer_cast<LiteralExpr>(binary->getLeft());
	auto right = std::dynamic_pointer_cast<LiteralExpr>(binary->getRight());
	ASSERT_NE(left, nullptr);
	ASSERT_NE(right, nullptr);
	EXPECT_EQ(left->getValue().getLexeme(), "3");
	EXPECT_EQ(right->getValue().getLexeme(), "5");
}
