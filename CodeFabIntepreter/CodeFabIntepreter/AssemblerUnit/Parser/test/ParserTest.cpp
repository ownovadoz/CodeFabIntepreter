#include "../Statement.h"
#include "../Expression.h"
#include "../Parser.h"
#include "../../Tokenizer/Token.h"
#include "../../../CodeFabException.h"

#include <gmock/gmock.h>
#include <memory>
#include <string>
#include <vector>

using std::string;
using std::unique_ptr;
using std::vector;
using namespace testing;

class ParserTestFixture : public Test {
protected:
	VarDeclareStmt* buildAndParseVarDeclareStmt(const vector<Token>& initializer_tokens) {
		vector<Token> tokens = {
			{TokenType::VAR, "var", "var", 1},
			{TokenType::IDENTIFIER, "a", "a", 1},
			{TokenType::EQUAL, "=", "=", 1},
		};
		tokens.insert(tokens.end(), initializer_tokens.begin(), initializer_tokens.end());
		tokens.push_back({ TokenType::SEMICOLON, ";", ";", 1 });
		tokens.push_back({ TokenType::END_OF_FILE, "\n", "\n", 1 });

		parsed = parser.parse(tokens);
		return dynamic_cast<VarDeclareStmt*>(parsed.get());
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

	void expectUnary(const Expression* expr, TokenType op_type, const string& op_lexeme, TokenType operand_type, const string& operand_lexeme) {
		const UnaryExpr* unary = dynamic_cast<const UnaryExpr*>(expr);

		ASSERT_NE(unary, nullptr);

		const Token& op = unary->getOperator();

		EXPECT_EQ(op.getType(), op_type);
		EXPECT_EQ(op.getLexeme(), op_lexeme);

		ASSERT_NO_FATAL_FAILURE(expectLiteral(unary->getExpr(), operand_type, operand_lexeme));
	}

	void expectParseThrows(const vector<Token>& tokens) {
		EXPECT_THROW(parser.parse(tokens), CodeFabException);
	}

	void expectGrouping(const Expression* expr) {
		const GroupingExpr* grouping = dynamic_cast<const GroupingExpr*>(expr);

		EXPECT_NE(grouping, nullptr);
	}

	void expectVariable(const Expression* expr, const string& lexeme) {
		const VariableExpr* variable = dynamic_cast<const VariableExpr*>(expr);

		ASSERT_NE(variable, nullptr);

		const Token& token = variable->getToken();

		EXPECT_EQ(token.getType(), TokenType::IDENTIFIER);
		EXPECT_EQ(token.getLexeme(), lexeme);
	}

	Parser parser;
	unique_ptr<Statement> parsed;
};

TEST_F(ParserTestFixture, VarDeclareStmtSingleNumberPassed) {
	VarDeclareStmt* stmt = buildAndParseVarDeclareStmt({ {TokenType::NUMBER, "10", 10.0, 1} });

	ASSERT_NO_FATAL_FAILURE(expectDeclaredName(stmt, "a"));
	ASSERT_NO_FATAL_FAILURE(expectLiteral(stmt->getInitializer(), TokenType::NUMBER, "10"));
}

TEST_F(ParserTestFixture, VarDeclareStmtSingleStringPassed) {
	VarDeclareStmt* stmt = buildAndParseVarDeclareStmt({ {TokenType::STRING, "\"text\"", "text", 1} });

	ASSERT_NO_FATAL_FAILURE(expectDeclaredName(stmt, "a"));
	ASSERT_NO_FATAL_FAILURE(expectLiteral(stmt->getInitializer(), TokenType::STRING, "\"text\""));
}

TEST_F(ParserTestFixture, VarDeclareStmtSingleTruePassed) {
	VarDeclareStmt* stmt = buildAndParseVarDeclareStmt({ {TokenType::TRUE, "true", true, 1} });

	ASSERT_NO_FATAL_FAILURE(expectDeclaredName(stmt, "a"));
	ASSERT_NO_FATAL_FAILURE(expectLiteral(stmt->getInitializer(), TokenType::TRUE, "true"));
}

TEST_F(ParserTestFixture, VarDeclareStmtSingleFalsePassed) {
	VarDeclareStmt* stmt = buildAndParseVarDeclareStmt({ {TokenType::FALSE, "false", false, 1} });

	ASSERT_NO_FATAL_FAILURE(expectDeclaredName(stmt, "a"));
	ASSERT_NO_FATAL_FAILURE(expectLiteral(stmt->getInitializer(), TokenType::FALSE, "false"));
}

TEST_F(ParserTestFixture, VarDeclareStmtNegativeNumberPassed) {
	VarDeclareStmt* stmt = buildAndParseVarDeclareStmt({
		{TokenType::MINUS, "-", "-", 1},
		{TokenType::NUMBER, "10", 10.0, 1}
	});

	ASSERT_NO_FATAL_FAILURE(expectDeclaredName(stmt, "a"));
	ASSERT_NO_FATAL_FAILURE(expectUnary(stmt->getInitializer(), TokenType::MINUS, "-", TokenType::NUMBER, "10"));
}

TEST_F(ParserTestFixture, VarDeclareStmtNegatedTruePassed) {
	VarDeclareStmt* stmt = buildAndParseVarDeclareStmt({
		{TokenType::BANG, "!", "!", 1},
		{TokenType::TRUE, "true", true, 1}
	});

	ASSERT_NO_FATAL_FAILURE(expectDeclaredName(stmt, "a"));
	ASSERT_NO_FATAL_FAILURE(expectUnary(stmt->getInitializer(), TokenType::BANG, "!", TokenType::TRUE, "true"));
}

TEST_F(ParserTestFixture, VarDeclareStmtNegatedFalsePassed) {
	VarDeclareStmt* stmt = buildAndParseVarDeclareStmt({
		{TokenType::BANG, "!", "!", 1},
		{TokenType::FALSE, "false", false, 1}
	});

	ASSERT_NO_FATAL_FAILURE(expectDeclaredName(stmt, "a"));
	ASSERT_NO_FATAL_FAILURE(expectUnary(stmt->getInitializer(), TokenType::BANG, "!", TokenType::FALSE, "false"));
}

TEST_F(ParserTestFixture, VarDeclareStmtGroupedNumberPassed) {
	VarDeclareStmt* stmt = buildAndParseVarDeclareStmt({
		{TokenType::LEFT_PAREN, "(", "(", 1},
		{TokenType::NUMBER, "10", 10.0, 1},
		{TokenType::RIGHT_PAREN, ")", ")", 1}
	});

	ASSERT_NO_FATAL_FAILURE(expectDeclaredName(stmt, "a"));
	ASSERT_NO_FATAL_FAILURE(expectGrouping(stmt->getInitializer()));
}

TEST_F(ParserTestFixture, VarDeclareStmtGroupedNegativeNumberPassed) {
	VarDeclareStmt* stmt = buildAndParseVarDeclareStmt({
		{TokenType::LEFT_PAREN, "(", "(", 1},
		{TokenType::MINUS, "-", "-", 1},
		{TokenType::NUMBER, "10", 10.0, 1},
		{TokenType::RIGHT_PAREN, ")", ")", 1}
	});

	ASSERT_NO_FATAL_FAILURE(expectDeclaredName(stmt, "a"));
	ASSERT_NO_FATAL_FAILURE(expectGrouping(stmt->getInitializer()));
}

TEST_F(ParserTestFixture, VarDeclareStmtNestedGroupingPassed) {
	VarDeclareStmt* stmt = buildAndParseVarDeclareStmt({
		{TokenType::LEFT_PAREN, "(", "(", 1},
		{TokenType::LEFT_PAREN, "(", "(", 1},
		{TokenType::NUMBER, "10", 10.0, 1},
		{TokenType::RIGHT_PAREN, ")", ")", 1},
		{TokenType::RIGHT_PAREN, ")", ")", 1}
	});

	ASSERT_NO_FATAL_FAILURE(expectDeclaredName(stmt, "a"));
	ASSERT_NO_FATAL_FAILURE(expectGrouping(stmt->getInitializer()));
}

TEST_F(ParserTestFixture, VarDeclareStmtGroupingMissingClosingParenFailed) {
	// var a = (10;
	expectParseThrows({
		{TokenType::VAR, "var", "var", 1},
		{TokenType::IDENTIFIER, "a", "a", 1},
		{TokenType::EQUAL, "=", "=", 1},
		{TokenType::LEFT_PAREN, "(", "(", 1},
		{TokenType::NUMBER, "10", 10.0, 1},
		{TokenType::SEMICOLON, ";", ";", 1},
		{TokenType::END_OF_FILE, "\n", "\n", 1}
	});
}

TEST_F(ParserTestFixture, VarDeclareStmtGroupingEmptyFailed) {
	// var a = ();
	expectParseThrows({
		{TokenType::VAR, "var", "var", 1},
		{TokenType::IDENTIFIER, "a", "a", 1},
		{TokenType::EQUAL, "=", "=", 1},
		{TokenType::LEFT_PAREN, "(", "(", 1},
		{TokenType::RIGHT_PAREN, ")", ")", 1},
		{TokenType::SEMICOLON, ";", ";", 1},
		{TokenType::END_OF_FILE, "\n", "\n", 1}
	});
}

TEST_F(ParserTestFixture, VarDeclareStmtGroupingTrailingExtraParenFailed) {
	// var a = (10));
	expectParseThrows({
		{TokenType::VAR, "var", "var", 1},
		{TokenType::IDENTIFIER, "a", "a", 1},
		{TokenType::EQUAL, "=", "=", 1},
		{TokenType::LEFT_PAREN, "(", "(", 1},
		{TokenType::NUMBER, "10", 10.0, 1},
		{TokenType::RIGHT_PAREN, ")", ")", 1},
		{TokenType::RIGHT_PAREN, ")", ")", 1},
		{TokenType::SEMICOLON, ";", ";", 1},
		{TokenType::END_OF_FILE, "\n", "\n", 1}
	});
}

TEST_F(ParserTestFixture, VarDeclareStmtVariableReferencePassed) {
	VarDeclareStmt* stmt = buildAndParseVarDeclareStmt({ {TokenType::IDENTIFIER, "b", "b", 1} });

	ASSERT_NO_FATAL_FAILURE(expectDeclaredName(stmt, "a"));
	ASSERT_NO_FATAL_FAILURE(expectVariable(stmt->getInitializer(), "b"));
}

TEST_F(ParserTestFixture, VarDeclareStmtNegatedVariablePassed) {
	VarDeclareStmt* stmt = buildAndParseVarDeclareStmt({
		{TokenType::MINUS, "-", "-", 1},
		{TokenType::IDENTIFIER, "b", "b", 1}
	});

	ASSERT_NO_FATAL_FAILURE(expectDeclaredName(stmt, "a"));

	const UnaryExpr* unary = dynamic_cast<const UnaryExpr*>(stmt->getInitializer());
	ASSERT_NE(unary, nullptr);
	EXPECT_EQ(unary->getOperator().getType(), TokenType::MINUS);
	ASSERT_NO_FATAL_FAILURE(expectVariable(unary->getExpr(), "b"));
}

TEST_F(ParserTestFixture, VarDeclareStmtGroupedVariablePassed) {
	VarDeclareStmt* stmt = buildAndParseVarDeclareStmt({
		{TokenType::LEFT_PAREN, "(", "(", 1},
		{TokenType::IDENTIFIER, "b", "b", 1},
		{TokenType::RIGHT_PAREN, ")", ")", 1}
	});

	ASSERT_NO_FATAL_FAILURE(expectDeclaredName(stmt, "a"));
	ASSERT_NO_FATAL_FAILURE(expectGrouping(stmt->getInitializer()));
}

TEST_F(ParserTestFixture, VarDeclareStmtMissingIdentifierFailed) {
	// var = 10;
	expectParseThrows({
		{TokenType::VAR, "var", "var", 1},
		{TokenType::EQUAL, "=", "=", 1},
		{TokenType::NUMBER, "10", 10.0, 1},
		{TokenType::SEMICOLON, ";", ";", 1},
		{TokenType::END_OF_FILE, "\n", "\n", 1}
	});
}

TEST_F(ParserTestFixture, VarDeclareStmtMissingEqualFailed) {
	// var a 10;
	expectParseThrows({
		{TokenType::VAR, "var", "var", 1},
		{TokenType::IDENTIFIER, "a", "a", 1},
		{TokenType::NUMBER, "10", 10.0, 1},
		{TokenType::SEMICOLON, ";", ";", 1},
		{TokenType::END_OF_FILE, "\n", "\n", 1}
	});
}

TEST_F(ParserTestFixture, VarDeclareStmtNoInitializerFailed) {
	// var a; — initializer-less var declarations are not supported yet
	expectParseThrows({
		{TokenType::VAR, "var", "var", 1},
		{TokenType::IDENTIFIER, "a", "a", 1},
		{TokenType::SEMICOLON, ";", ";", 1},
		{TokenType::END_OF_FILE, "\n", "\n", 1}
	});
}

TEST_F(ParserTestFixture, VarDeclareStmtMissingInitializerFailed) {
	// var a = ;
	expectParseThrows({
		{TokenType::VAR, "var", "var", 1},
		{TokenType::IDENTIFIER, "a", "a", 1},
		{TokenType::EQUAL, "=", "=", 1},
		{TokenType::SEMICOLON, ";", ";", 1},
		{TokenType::END_OF_FILE, "\n", "\n", 1}
	});
}

TEST_F(ParserTestFixture, VarDeclareStmtMissingSemicolonFailed) {
	// var a = 10
	expectParseThrows({
		{TokenType::VAR, "var", "var", 1},
		{TokenType::IDENTIFIER, "a", "a", 1},
		{TokenType::EQUAL, "=", "=", 1},
		{TokenType::NUMBER, "10", 10.0, 1},
		{TokenType::END_OF_FILE, "\n", "\n", 1}
	});
}

TEST_F(ParserTestFixture, VarDeclareStmtDanglingMinusFailed) {
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

TEST_F(ParserTestFixture, VarDeclareStmtDanglingBangFailed) {
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

TEST_F(ParserTestFixture, VarDeclareStmtDanglingMinusWithExtraSemicolonFailed) {
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
