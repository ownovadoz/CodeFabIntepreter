#include "../Statement.h"
#include "../Expression.h"
#include "../Parser.h"
#include "../../Tokenizer/Token.h"

#include <gmock/gmock.h>
#include <exception>
#include <string>
#include <vector>

using std::exception;
using std::string;
using std::vector;
using namespace testing;

class ParserTest : public Test {
protected:
	void TearDown() override {
		delete stmt;
	}

	VarDeclareStmt* parseVarDeclareStmt(const vector<Token>& initializerTokens) {
		vector<Token> tokens = {
			{TokenType::VAR, "var", "var", 1},
			{TokenType::IDENTIFIER, "a", "a", 1},
			{TokenType::EQUAL, "=", "=", 1},
		};
		tokens.insert(tokens.end(), initializerTokens.begin(), initializerTokens.end());
		tokens.push_back({ TokenType::SEMICOLON, ";", ";", 1 });
		tokens.push_back({ TokenType::END_OF_FILE, "\n", "\n", 1 });

		stmt = dynamic_cast<VarDeclareStmt*>(parser.parse(tokens));
		return stmt;
	}

	void expectDeclaredName(VarDeclareStmt* stmt, const string& lexeme) {
		ASSERT_NE(stmt, nullptr);

		const Token& name = stmt->getName();

		EXPECT_EQ(name.getType(), TokenType::IDENTIFIER);
		EXPECT_EQ(name.getLexeme(), lexeme);
	}

	void expectLiteral(const Expression* expr, TokenType type, const string& lexeme) {
		const LiteralExpr* literal = dynamic_cast<const LiteralExpr*>(expr);

		ASSERT_NE(literal, nullptr);

		const Token& token = literal->getToken();

		EXPECT_EQ(token.getType(), type);
		EXPECT_EQ(token.getLexeme(), lexeme);
		EXPECT_EQ(token.getLine(), 1);
	}

	void expectUnary(const Expression* expr, TokenType opType, const string& opLexeme, TokenType operandType, const string& operandLexeme) {
		const UnaryExpr* unary = dynamic_cast<const UnaryExpr*>(expr);

		ASSERT_NE(unary, nullptr);

		const Token& op = unary->getOperator();

		EXPECT_EQ(op.getType(), opType);
		EXPECT_EQ(op.getLexeme(), opLexeme);

		ASSERT_NO_FATAL_FAILURE(expectLiteral(unary->getExpr(), operandType, operandLexeme));
	}

	void expectParseThrows(const vector<Token>& tokens) {
		EXPECT_THROW(parser.parse(tokens), exception);
	}

	Parser parser;
	VarDeclareStmt* stmt = nullptr;
};

TEST_F(ParserTest, VarDeclareStmtSingleNumberPassed) {
	VarDeclareStmt* stmt = parseVarDeclareStmt({ {TokenType::NUMBER, "10", 10.0, 1} });

	ASSERT_NO_FATAL_FAILURE(expectDeclaredName(stmt, "a"));
	ASSERT_NO_FATAL_FAILURE(expectLiteral(stmt->getInitializer(), TokenType::NUMBER, "10"));
}

TEST_F(ParserTest, VarDeclareStmtSingleStringPassed) {
	VarDeclareStmt* stmt = parseVarDeclareStmt({ {TokenType::STRING, "\"text\"", "text", 1} });

	ASSERT_NO_FATAL_FAILURE(expectDeclaredName(stmt, "a"));
	ASSERT_NO_FATAL_FAILURE(expectLiteral(stmt->getInitializer(), TokenType::STRING, "\"text\""));
}

TEST_F(ParserTest, VarDeclareStmtSingleTruePassed) {
	VarDeclareStmt* stmt = parseVarDeclareStmt({ {TokenType::TRUE, "true", true, 1} });

	ASSERT_NO_FATAL_FAILURE(expectDeclaredName(stmt, "a"));
	ASSERT_NO_FATAL_FAILURE(expectLiteral(stmt->getInitializer(), TokenType::TRUE, "true"));
}

TEST_F(ParserTest, VarDeclareStmtSingleFalsePassed) {
	VarDeclareStmt* stmt = parseVarDeclareStmt({ {TokenType::FALSE, "false", false, 1} });

	ASSERT_NO_FATAL_FAILURE(expectDeclaredName(stmt, "a"));
	ASSERT_NO_FATAL_FAILURE(expectLiteral(stmt->getInitializer(), TokenType::FALSE, "false"));
}

TEST_F(ParserTest, VarDeclareStmtNegativeNumberPassed) {
	VarDeclareStmt* stmt = parseVarDeclareStmt({
		{TokenType::MINUS, "-", "-", 1},
		{TokenType::NUMBER, "10", 10.0, 1}
	});

	ASSERT_NO_FATAL_FAILURE(expectDeclaredName(stmt, "a"));
	ASSERT_NO_FATAL_FAILURE(expectUnary(stmt->getInitializer(), TokenType::MINUS, "-", TokenType::NUMBER, "10"));
}

TEST_F(ParserTest, VarDeclareStmtNegatedTruePassed) {
	VarDeclareStmt* stmt = parseVarDeclareStmt({
		{TokenType::BANG, "!", "!", 1},
		{TokenType::TRUE, "true", true, 1}
	});

	ASSERT_NO_FATAL_FAILURE(expectDeclaredName(stmt, "a"));
	ASSERT_NO_FATAL_FAILURE(expectUnary(stmt->getInitializer(), TokenType::BANG, "!", TokenType::TRUE, "true"));
}

TEST_F(ParserTest, VarDeclareStmtNegatedFalsePassed) {
	VarDeclareStmt* stmt = parseVarDeclareStmt({
		{TokenType::BANG, "!", "!", 1},
		{TokenType::FALSE, "false", false, 1}
	});

	ASSERT_NO_FATAL_FAILURE(expectDeclaredName(stmt, "a"));
	ASSERT_NO_FATAL_FAILURE(expectUnary(stmt->getInitializer(), TokenType::BANG, "!", TokenType::FALSE, "false"));
}

TEST_F(ParserTest, VarDeclareStmtMissingIdentifierFailed) {
	// var = 10;
	expectParseThrows({
		{TokenType::VAR, "var", "var", 1},
		{TokenType::EQUAL, "=", "=", 1},
		{TokenType::NUMBER, "10", 10.0, 1},
		{TokenType::SEMICOLON, ";", ";", 1},
		{TokenType::END_OF_FILE, "\n", "\n", 1}
	});
}

TEST_F(ParserTest, VarDeclareStmtMissingEqualFailed) {
	// var a 10;
	expectParseThrows({
		{TokenType::VAR, "var", "var", 1},
		{TokenType::IDENTIFIER, "a", "a", 1},
		{TokenType::NUMBER, "10", 10.0, 1},
		{TokenType::SEMICOLON, ";", ";", 1},
		{TokenType::END_OF_FILE, "\n", "\n", 1}
	});
}

TEST_F(ParserTest, VarDeclareStmtNoInitializerFailed) {
	// var a; — initializer-less var declarations are not supported yet
	expectParseThrows({
		{TokenType::VAR, "var", "var", 1},
		{TokenType::IDENTIFIER, "a", "a", 1},
		{TokenType::SEMICOLON, ";", ";", 1},
		{TokenType::END_OF_FILE, "\n", "\n", 1}
	});
}

TEST_F(ParserTest, VarDeclareStmtMissingInitializerFailed) {
	// var a = ;
	expectParseThrows({
		{TokenType::VAR, "var", "var", 1},
		{TokenType::IDENTIFIER, "a", "a", 1},
		{TokenType::EQUAL, "=", "=", 1},
		{TokenType::SEMICOLON, ";", ";", 1},
		{TokenType::END_OF_FILE, "\n", "\n", 1}
	});
}

TEST_F(ParserTest, VarDeclareStmtMissingSemicolonFailed) {
	// var a = 10
	expectParseThrows({
		{TokenType::VAR, "var", "var", 1},
		{TokenType::IDENTIFIER, "a", "a", 1},
		{TokenType::EQUAL, "=", "=", 1},
		{TokenType::NUMBER, "10", 10.0, 1},
		{TokenType::END_OF_FILE, "\n", "\n", 1}
	});
}

TEST_F(ParserTest, VarDeclareStmtDanglingMinusFailed) {
	// var a = -;
	expectParseThrows({
		{TokenType::VAR, "var", "var", 1},
		{TokenType::IDENTIFIER, "a", "a", 1},
		{TokenType::EQUAL, "=", "=", 1},
		{TokenType::MINUS, "-", "-", 1},
		{TokenType::SEMICOLON, ";", ";", 1},
		{TokenType::END_OF_FILE, "\n", "\n", 1}
	});
}

TEST_F(ParserTest, VarDeclareStmtDanglingBangFailed) {
	// var a = !;
	expectParseThrows({
		{TokenType::VAR, "var", "var", 1},
		{TokenType::IDENTIFIER, "a", "a", 1},
		{TokenType::EQUAL, "=", "=", 1},
		{TokenType::BANG, "!", "!", 1},
		{TokenType::SEMICOLON, ";", ";", 1},
		{TokenType::END_OF_FILE, "\n", "\n", 1}
	});
}

TEST_F(ParserTest, VarDeclareStmtDanglingMinusWithExtraSemicolonFailed) {
	// var a = -;; — an extra SEMICOLON must not let a missing operand slip through
	// as if it were the statement's real terminator.
	expectParseThrows({
		{TokenType::VAR, "var", "var", 1},
		{TokenType::IDENTIFIER, "a", "a", 1},
		{TokenType::EQUAL, "=", "=", 1},
		{TokenType::MINUS, "-", "-", 1},
		{TokenType::SEMICOLON, ";", ";", 1},
		{TokenType::SEMICOLON, ";", ";", 1},
		{TokenType::END_OF_FILE, "\n", "\n", 1}
	});
}
