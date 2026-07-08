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