#include "Statement.h"
#include "Expression.h"
#include "Parser.h"
#include "../Tokenizer/Token.h"
#include "../../CodeFabException.h"

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
		return parsed.empty() ? nullptr : dynamic_cast<VarDeclareStmt*>(parsed[0].get());
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

	const Expression* expectAssignAndGetValue(const Expression* expr, const string& identifier_lexeme) {
		const AssignExpr* assign = dynamic_cast<const AssignExpr*>(expr);

		EXPECT_NE(assign, nullptr);
		if (assign == nullptr) return nullptr;

		EXPECT_EQ(assign->getIdentifier().getType(), TokenType::IDENTIFIER);
		EXPECT_EQ(assign->getIdentifier().getLexeme(), identifier_lexeme);

		return assign->getValue();
	}

	void expectBinary(const Expression* expr, TokenType op_type, const string& op_lexeme) {
		const BinaryExpr* binary = dynamic_cast<const BinaryExpr*>(expr);

		EXPECT_NE(binary, nullptr);
		if (binary == nullptr) return;

		EXPECT_EQ(binary->getOperator().getType(), op_type);
		EXPECT_EQ(binary->getOperator().getLexeme(), op_lexeme);
	}

	void expectLogical(const Expression* expr, TokenType op_type, const string& op_lexeme) {
		const LogicalExpr* logical = dynamic_cast<const LogicalExpr*>(expr);

		EXPECT_NE(logical, nullptr);
		if (logical == nullptr) return;

		EXPECT_EQ(logical->getOperator().getType(), op_type);
		EXPECT_EQ(logical->getOperator().getLexeme(), op_lexeme);
	}

	const vector<unique_ptr<Statement>>& buildAndParseProgram(const vector<Token>& tokens) {
		parsed = parser.parse(tokens);
		return parsed;
	}

	Parser parser;
	vector<unique_ptr<Statement>> parsed;
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

TEST_F(ParserTestFixture, VarDeclareStmtAssignToLiteralPassed) {
	VarDeclareStmt* stmt = buildAndParseVarDeclareStmt({
		{TokenType::IDENTIFIER, "b", "b", 1},
		{TokenType::EQUAL, "=", "=", 1},
		{TokenType::NUMBER, "10", 10.0, 1}
	});

	ASSERT_NO_FATAL_FAILURE(expectDeclaredName(stmt, "a"));

	const Expression* value = expectAssignAndGetValue(stmt->getInitializer(), "b");
	ASSERT_NO_FATAL_FAILURE(expectLiteral(value, TokenType::NUMBER, "10"));
}

TEST_F(ParserTestFixture, VarDeclareStmtAssignToVariablePassed) {
	VarDeclareStmt* stmt = buildAndParseVarDeclareStmt({
		{TokenType::IDENTIFIER, "b", "b", 1},
		{TokenType::EQUAL, "=", "=", 1},
		{TokenType::IDENTIFIER, "c", "c", 1}
	});

	ASSERT_NO_FATAL_FAILURE(expectDeclaredName(stmt, "a"));

	const Expression* value = expectAssignAndGetValue(stmt->getInitializer(), "b");
	ASSERT_NO_FATAL_FAILURE(expectVariable(value, "c"));
}

TEST_F(ParserTestFixture, VarDeclareStmtChainedAssignPassed) {
	VarDeclareStmt* stmt = buildAndParseVarDeclareStmt({
		{TokenType::IDENTIFIER, "b", "b", 1},
		{TokenType::EQUAL, "=", "=", 1},
		{TokenType::IDENTIFIER, "c", "c", 1},
		{TokenType::EQUAL, "=", "=", 1},
		{TokenType::NUMBER, "10", 10.0, 1}
	});

	ASSERT_NO_FATAL_FAILURE(expectDeclaredName(stmt, "a"));

	const Expression* outer_value = expectAssignAndGetValue(stmt->getInitializer(), "b");
	const Expression* inner_value = expectAssignAndGetValue(outer_value, "c");
	ASSERT_NO_FATAL_FAILURE(expectLiteral(inner_value, TokenType::NUMBER, "10"));
}

TEST_F(ParserTestFixture, VarDeclareStmtInvalidAssignmentTargetFailed) {
	// var a = 10 = 20;
	expectParseThrows({
		{TokenType::VAR, "var", "var", 1},
		{TokenType::IDENTIFIER, "a", "a", 1},
		{TokenType::EQUAL, "=", "=", 1},
		{TokenType::NUMBER, "10", 10.0, 1},
		{TokenType::EQUAL, "=", "=", 1},
		{TokenType::NUMBER, "20", 20.0, 1},
		{TokenType::SEMICOLON, ";", ";", 1},
		{TokenType::END_OF_FILE, "\n", "\n", 1}
	});
}

TEST_F(ParserTestFixture, VarDeclareStmtAssignMissingValueFailed) {
	// var a = b = ;
	expectParseThrows({
		{TokenType::VAR, "var", "var", 1},
		{TokenType::IDENTIFIER, "a", "a", 1},
		{TokenType::EQUAL, "=", "=", 1},
		{TokenType::IDENTIFIER, "b", "b", 1},
		{TokenType::EQUAL, "=", "=", 1},
		{TokenType::SEMICOLON, ";", ";", 1},
		{TokenType::END_OF_FILE, "\n", "\n", 1}
	});
}

TEST_F(ParserTestFixture, VarDeclareStmtBinaryAdditionPassed) {
	VarDeclareStmt* stmt = buildAndParseVarDeclareStmt({
		{TokenType::NUMBER, "1", 1.0, 1},
		{TokenType::PLUS, "+", "+", 1},
		{TokenType::NUMBER, "2", 2.0, 1}
	});

	ASSERT_NO_FATAL_FAILURE(expectDeclaredName(stmt, "a"));
	ASSERT_NO_FATAL_FAILURE(expectBinary(stmt->getInitializer(), TokenType::PLUS, "+"));
}

TEST_F(ParserTestFixture, VarDeclareStmtBinaryMultiplicationPassed) {
	VarDeclareStmt* stmt = buildAndParseVarDeclareStmt({
		{TokenType::NUMBER, "1", 1.0, 1},
		{TokenType::STAR, "*", "*", 1},
		{TokenType::NUMBER, "2", 2.0, 1}
	});

	ASSERT_NO_FATAL_FAILURE(expectDeclaredName(stmt, "a"));
	ASSERT_NO_FATAL_FAILURE(expectBinary(stmt->getInitializer(), TokenType::STAR, "*"));
}

TEST_F(ParserTestFixture, VarDeclareStmtBinaryPrecedencePassed) {
	// 1 + 2 * 3 should parse as PLUS(1, STAR(2, 3)) once precedence climbing exists
	VarDeclareStmt* stmt = buildAndParseVarDeclareStmt({
		{TokenType::NUMBER, "1", 1.0, 1},
		{TokenType::PLUS, "+", "+", 1},
		{TokenType::NUMBER, "2", 2.0, 1},
		{TokenType::STAR, "*", "*", 1},
		{TokenType::NUMBER, "3", 3.0, 1}
	});

	ASSERT_NO_FATAL_FAILURE(expectDeclaredName(stmt, "a"));
	ASSERT_NO_FATAL_FAILURE(expectBinary(stmt->getInitializer(), TokenType::PLUS, "+"));
}

TEST_F(ParserTestFixture, VarDeclareStmtBinaryMissingRightOperandFailed) {
	// var a = 1 + ;
	expectParseThrows({
		{TokenType::VAR, "var", "var", 1},
		{TokenType::IDENTIFIER, "a", "a", 1},
		{TokenType::EQUAL, "=", "=", 1},
		{TokenType::NUMBER, "1", 1.0, 1},
		{TokenType::PLUS, "+", "+", 1},
		{TokenType::SEMICOLON, ";", ";", 1},
		{TokenType::END_OF_FILE, "\n", "\n", 1}
	});
}

TEST_F(ParserTestFixture, VarDeclareStmtLogicalAndPassed) {
	VarDeclareStmt* stmt = buildAndParseVarDeclareStmt({
		{TokenType::TRUE, "true", true, 1},
		{TokenType::AND, "&&", "&&", 1},
		{TokenType::FALSE, "false", false, 1}
	});

	ASSERT_NO_FATAL_FAILURE(expectDeclaredName(stmt, "a"));
	ASSERT_NO_FATAL_FAILURE(expectLogical(stmt->getInitializer(), TokenType::AND, "&&"));
}

TEST_F(ParserTestFixture, VarDeclareStmtLogicalOrPassed) {
	VarDeclareStmt* stmt = buildAndParseVarDeclareStmt({
		{TokenType::TRUE, "true", true, 1},
		{TokenType::OR, "||", "||", 1},
		{TokenType::FALSE, "false", false, 1}
	});

	ASSERT_NO_FATAL_FAILURE(expectDeclaredName(stmt, "a"));
	ASSERT_NO_FATAL_FAILURE(expectLogical(stmt->getInitializer(), TokenType::OR, "||"));
}

TEST_F(ParserTestFixture, VarDeclareStmtLogicalMissingRightOperandFailed) {
	// var a = true && ;
	expectParseThrows({
		{TokenType::VAR, "var", "var", 1},
		{TokenType::IDENTIFIER, "a", "a", 1},
		{TokenType::EQUAL, "=", "=", 1},
		{TokenType::TRUE, "true", true, 1},
		{TokenType::AND, "&&", "&&", 1},
		{TokenType::SEMICOLON, ";", ";", 1},
		{TokenType::END_OF_FILE, "\n", "\n", 1}
	});
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

TEST_F(ParserTestFixture, ExpressionStmtAssignmentPassed) {
	// a = 10;
	const auto& program = buildAndParseProgram({
		{TokenType::IDENTIFIER, "a", "a", 1},
		{TokenType::EQUAL, "=", "=", 1},
		{TokenType::NUMBER, "10", 10.0, 1},
		{TokenType::SEMICOLON, ";", ";", 1},
		{TokenType::END_OF_FILE, "\n", "\n", 1}
	});

	ASSERT_EQ(program.size(), 1);
	const ExpressionStmt* expr_stmt = dynamic_cast<const ExpressionStmt*>(program[0].get());
	EXPECT_NE(expr_stmt, nullptr);
}

TEST_F(ParserTestFixture, BareSemicolonProducesNoStatement) {
	// ;
	const auto& program = buildAndParseProgram({
		{TokenType::SEMICOLON, ";", ";", 1},
		{TokenType::END_OF_FILE, "\n", "\n", 1}
	});

	ASSERT_EQ(program.size(), 1);
	EXPECT_EQ(program[0].get(), nullptr);
}

TEST_F(ParserTestFixture, ExpressionStmtMissingSemicolonFailed) {
	// a = 10
	expectParseThrows({
		{TokenType::IDENTIFIER, "a", "a", 1},
		{TokenType::EQUAL, "=", "=", 1},
		{TokenType::NUMBER, "10", 10.0, 1},
		{TokenType::END_OF_FILE, "\n", "\n", 1}
	});
}

TEST_F(ParserTestFixture, MultiStatementInvalidAssignmentTargetFailed) {
	// var a = 1; var b = 2; a + b = 3;
	expectParseThrows({
		{TokenType::VAR, "var", "var", 1},
		{TokenType::IDENTIFIER, "a", "a", 1},
		{TokenType::EQUAL, "=", "=", 1},
		{TokenType::NUMBER, "1", 1.0, 1},
		{TokenType::SEMICOLON, ";", ";", 1},
		{TokenType::VAR, "var", "var", 1},
		{TokenType::IDENTIFIER, "b", "b", 1},
		{TokenType::EQUAL, "=", "=", 1},
		{TokenType::NUMBER, "2", 2.0, 1},
		{TokenType::SEMICOLON, ";", ";", 1},
		{TokenType::IDENTIFIER, "a", "a", 1},
		{TokenType::PLUS, "+", "+", 1},
		{TokenType::IDENTIFIER, "b", "b", 1},
		{TokenType::EQUAL, "=", "=", 1},
		{TokenType::NUMBER, "3", 3.0, 1},
		{TokenType::SEMICOLON, ";", ";", 1},
		{TokenType::END_OF_FILE, "\n", "\n", 1}
	});
}

TEST_F(ParserTestFixture, PrintStmtPassed) {
	// print 10;
	const auto& program = buildAndParseProgram({
		{TokenType::PRINT, "print", "print", 1},
		{TokenType::NUMBER, "10", 10.0, 1},
		{TokenType::SEMICOLON, ";", ";", 1},
		{TokenType::END_OF_FILE, "\n", "\n", 1}
	});

	ASSERT_EQ(program.size(), 1);
	const PrintStmt* print_stmt = dynamic_cast<const PrintStmt*>(program[0].get());
	EXPECT_NE(print_stmt, nullptr);
}

TEST_F(ParserTestFixture, PrintStmtMissingExpressionFailed) {
	// print;
	expectParseThrows({
		{TokenType::PRINT, "print", "print", 1},
		{TokenType::SEMICOLON, ";", ";", 1},
		{TokenType::END_OF_FILE, "\n", "\n", 1}
	});
}

TEST_F(ParserTestFixture, IfStmtWithElsePassed) {
	// if (a) print b; else print c;
	const auto& program = buildAndParseProgram({
		{TokenType::IF, "if", "if", 1},
		{TokenType::LEFT_PAREN, "(", "(", 1},
		{TokenType::IDENTIFIER, "a", "a", 1},
		{TokenType::RIGHT_PAREN, ")", ")", 1},
		{TokenType::PRINT, "print", "print", 1},
		{TokenType::IDENTIFIER, "b", "b", 1},
		{TokenType::SEMICOLON, ";", ";", 1},
		{TokenType::ELSE, "else", "else", 1},
		{TokenType::PRINT, "print", "print", 1},
		{TokenType::IDENTIFIER, "c", "c", 1},
		{TokenType::SEMICOLON, ";", ";", 1},
		{TokenType::END_OF_FILE, "\n", "\n", 1}
	});

	ASSERT_EQ(program.size(), 1);
	const IfStmt* if_stmt = dynamic_cast<const IfStmt*>(program[0].get());
	EXPECT_NE(if_stmt, nullptr);
}

TEST_F(ParserTestFixture, IfStmtWithoutElsePassed) {
	// if (a) print b;
	const auto& program = buildAndParseProgram({
		{TokenType::IF, "if", "if", 1},
		{TokenType::LEFT_PAREN, "(", "(", 1},
		{TokenType::IDENTIFIER, "a", "a", 1},
		{TokenType::RIGHT_PAREN, ")", ")", 1},
		{TokenType::PRINT, "print", "print", 1},
		{TokenType::IDENTIFIER, "b", "b", 1},
		{TokenType::SEMICOLON, ";", ";", 1},
		{TokenType::END_OF_FILE, "\n", "\n", 1}
	});

	ASSERT_EQ(program.size(), 1);
	const IfStmt* if_stmt = dynamic_cast<const IfStmt*>(program[0].get());
	EXPECT_NE(if_stmt, nullptr);
}

TEST_F(ParserTestFixture, IfStmtMissingParenFailed) {
	// if a) print b;
	expectParseThrows({
		{TokenType::IF, "if", "if", 1},
		{TokenType::IDENTIFIER, "a", "a", 1},
		{TokenType::RIGHT_PAREN, ")", ")", 1},
		{TokenType::PRINT, "print", "print", 1},
		{TokenType::IDENTIFIER, "b", "b", 1},
		{TokenType::SEMICOLON, ";", ";", 1},
		{TokenType::END_OF_FILE, "\n", "\n", 1}
	});
}

TEST_F(ParserTestFixture, ForStmtFullClausePassed) {
	// for (var i = 0; i < 10; i = i + 1) print i;
	const auto& program = buildAndParseProgram({
		{TokenType::FOR, "for", "for", 1},
		{TokenType::LEFT_PAREN, "(", "(", 1},
		{TokenType::VAR, "var", "var", 1},
		{TokenType::IDENTIFIER, "i", "i", 1},
		{TokenType::EQUAL, "=", "=", 1},
		{TokenType::NUMBER, "0", 0.0, 1},
		{TokenType::SEMICOLON, ";", ";", 1},
		{TokenType::IDENTIFIER, "i", "i", 1},
		{TokenType::LESS, "<", "<", 1},
		{TokenType::NUMBER, "10", 10.0, 1},
		{TokenType::SEMICOLON, ";", ";", 1},
		{TokenType::IDENTIFIER, "i", "i", 1},
		{TokenType::EQUAL, "=", "=", 1},
		{TokenType::IDENTIFIER, "i", "i", 1},
		{TokenType::PLUS, "+", "+", 1},
		{TokenType::NUMBER, "1", 1.0, 1},
		{TokenType::RIGHT_PAREN, ")", ")", 1},
		{TokenType::PRINT, "print", "print", 1},
		{TokenType::IDENTIFIER, "i", "i", 1},
		{TokenType::SEMICOLON, ";", ";", 1},
		{TokenType::END_OF_FILE, "\n", "\n", 1}
	});

	ASSERT_EQ(program.size(), 1);
	const ForStmt* for_stmt = dynamic_cast<const ForStmt*>(program[0].get());
	EXPECT_NE(for_stmt, nullptr);
}

TEST_F(ParserTestFixture, ForStmtMissingParenFailed) {
	// for var i = 0; ; print i;
	expectParseThrows({
		{TokenType::FOR, "for", "for", 1},
		{TokenType::VAR, "var", "var", 1},
		{TokenType::IDENTIFIER, "i", "i", 1},
		{TokenType::EQUAL, "=", "=", 1},
		{TokenType::NUMBER, "0", 0.0, 1},
		{TokenType::SEMICOLON, ";", ";", 1},
		{TokenType::SEMICOLON, ";", ";", 1},
		{TokenType::PRINT, "print", "print", 1},
		{TokenType::IDENTIFIER, "i", "i", 1},
		{TokenType::SEMICOLON, ";", ";", 1},
		{TokenType::END_OF_FILE, "\n", "\n", 1}
	});
}

TEST_F(ParserTestFixture, BlockStmtMultipleStatementsPassed) {
	// { print a; print b; }
	const auto& program = buildAndParseProgram({
		{TokenType::LEFT_BRACE, "{", "{", 1},
		{TokenType::PRINT, "print", "print", 1},
		{TokenType::IDENTIFIER, "a", "a", 1},
		{TokenType::SEMICOLON, ";", ";", 1},
		{TokenType::PRINT, "print", "print", 1},
		{TokenType::IDENTIFIER, "b", "b", 1},
		{TokenType::SEMICOLON, ";", ";", 1},
		{TokenType::RIGHT_BRACE, "}", "}", 1},
		{TokenType::END_OF_FILE, "\n", "\n", 1}
	});

	ASSERT_EQ(program.size(), 1);
	const BlockStmt* block_stmt = dynamic_cast<const BlockStmt*>(program[0].get());
	EXPECT_NE(block_stmt, nullptr);
}

TEST_F(ParserTestFixture, BlockStmtMissingClosingBraceFailed) {
	// { print a;
	expectParseThrows({
		{TokenType::LEFT_BRACE, "{", "{", 1},
		{TokenType::PRINT, "print", "print", 1},
		{TokenType::IDENTIFIER, "a", "a", 1},
		{TokenType::SEMICOLON, ";", ";", 1},
		{TokenType::END_OF_FILE, "\n", "\n", 1}
	});
}

TEST_F(ParserTestFixture, FunctionStmtWithParametersPassed) {
	// Func add(a, b) { return a + b; }
	const auto& program = buildAndParseProgram({
		{TokenType::FUNC, "Func", "Func", 1},
		{TokenType::IDENTIFIER, "add", "add", 1},
		{TokenType::LEFT_PAREN, "(", "(", 1},
		{TokenType::IDENTIFIER, "a", "a", 1},
		{TokenType::COMMA, ",", ",", 1},
		{TokenType::IDENTIFIER, "b", "b", 1},
		{TokenType::RIGHT_PAREN, ")", ")", 1},
		{TokenType::LEFT_BRACE, "{", "{", 1},
		{TokenType::RETURN, "return", "return", 1},
		{TokenType::IDENTIFIER, "a", "a", 1},
		{TokenType::PLUS, "+", "+", 1},
		{TokenType::IDENTIFIER, "b", "b", 1},
		{TokenType::SEMICOLON, ";", ";", 1},
		{TokenType::RIGHT_BRACE, "}", "}", 1},
		{TokenType::END_OF_FILE, "\n", "\n", 1}
	});

	ASSERT_EQ(program.size(), 1);
	const FunctionStmt* function_stmt = dynamic_cast<const FunctionStmt*>(program[0].get());
	ASSERT_NE(function_stmt, nullptr);
	EXPECT_EQ(function_stmt->getName().getLexeme(), "add");
	ASSERT_EQ(function_stmt->getParams().size(), 2u);
	EXPECT_EQ(function_stmt->getParams()[0].getLexeme(), "a");
	EXPECT_EQ(function_stmt->getParams()[1].getLexeme(), "b");
	ASSERT_EQ(function_stmt->getBody()->getStatements().size(), 1u);
	EXPECT_NE(dynamic_cast<const ReturnStmt*>(function_stmt->getBody()->getStatements()[0].get()), nullptr);
}

TEST_F(ParserTestFixture, FunctionStmtWithoutParametersPassed) {
	// Func noop() { }
	const auto& program = buildAndParseProgram({
		{TokenType::FUNC, "Func", "Func", 1},
		{TokenType::IDENTIFIER, "noop", "noop", 1},
		{TokenType::LEFT_PAREN, "(", "(", 1},
		{TokenType::RIGHT_PAREN, ")", ")", 1},
		{TokenType::LEFT_BRACE, "{", "{", 1},
		{TokenType::RIGHT_BRACE, "}", "}", 1},
		{TokenType::END_OF_FILE, "\n", "\n", 1}
	});

	ASSERT_EQ(program.size(), 1);
	const FunctionStmt* function_stmt = dynamic_cast<const FunctionStmt*>(program[0].get());
	ASSERT_NE(function_stmt, nullptr);
	EXPECT_TRUE(function_stmt->getParams().empty());
	EXPECT_TRUE(function_stmt->getBody()->getStatements().empty());
}

TEST_F(ParserTestFixture, FunctionStmtDuplicateParameterNameIsParsed) {
	// Func foo(a, a) { } — 파라미터 이름 중복 검사는 Checker의 책임이며,
	// Parser는 문법적으로 유효하므로 그대로 파싱에 성공해야 한다.
	const auto& program = buildAndParseProgram({
		{TokenType::FUNC, "Func", "Func", 1},
		{TokenType::IDENTIFIER, "foo", "foo", 1},
		{TokenType::LEFT_PAREN, "(", "(", 1},
		{TokenType::IDENTIFIER, "a", "a", 1},
		{TokenType::COMMA, ",", ",", 1},
		{TokenType::IDENTIFIER, "a", "a", 1},
		{TokenType::RIGHT_PAREN, ")", ")", 1},
		{TokenType::LEFT_BRACE, "{", "{", 1},
		{TokenType::RIGHT_BRACE, "}", "}", 1},
		{TokenType::END_OF_FILE, "\n", "\n", 1}
	});

	ASSERT_EQ(program.size(), 1);
	const FunctionStmt* function_stmt = dynamic_cast<const FunctionStmt*>(program[0].get());
	ASSERT_NE(function_stmt, nullptr);
	ASSERT_EQ(function_stmt->getParams().size(), 2u);
}

TEST_F(ParserTestFixture, FunctionStmtMissingNameFailed) {
	// Func (a) { }
	expectParseThrows({
		{TokenType::FUNC, "Func", "Func", 1},
		{TokenType::LEFT_PAREN, "(", "(", 1},
		{TokenType::IDENTIFIER, "a", "a", 1},
		{TokenType::RIGHT_PAREN, ")", ")", 1},
		{TokenType::LEFT_BRACE, "{", "{", 1},
		{TokenType::RIGHT_BRACE, "}", "}", 1},
		{TokenType::END_OF_FILE, "\n", "\n", 1}
	});
}

TEST_F(ParserTestFixture, FunctionStmtMissingBodyFailed) {
	// Func foo()
	expectParseThrows({
		{TokenType::FUNC, "Func", "Func", 1},
		{TokenType::IDENTIFIER, "foo", "foo", 1},
		{TokenType::LEFT_PAREN, "(", "(", 1},
		{TokenType::RIGHT_PAREN, ")", ")", 1},
		{TokenType::END_OF_FILE, "\n", "\n", 1}
	});
}

TEST_F(ParserTestFixture, ReturnStmtWithValuePassed) {
	// return 1;
	const auto& program = buildAndParseProgram({
		{TokenType::RETURN, "return", "return", 1},
		{TokenType::NUMBER, "1", 1.0, 1},
		{TokenType::SEMICOLON, ";", ";", 1},
		{TokenType::END_OF_FILE, "\n", "\n", 1}
	});

	ASSERT_EQ(program.size(), 1);
	const ReturnStmt* return_stmt = dynamic_cast<const ReturnStmt*>(program[0].get());
	ASSERT_NE(return_stmt, nullptr);
	ASSERT_NO_FATAL_FAILURE(expectLiteral(return_stmt->getValue(), TokenType::NUMBER, "1"));
}

TEST_F(ParserTestFixture, ReturnStmtWithoutValuePassed) {
	// return;
	const auto& program = buildAndParseProgram({
		{TokenType::RETURN, "return", "return", 1},
		{TokenType::SEMICOLON, ";", ";", 1},
		{TokenType::END_OF_FILE, "\n", "\n", 1}
	});

	ASSERT_EQ(program.size(), 1);
	const ReturnStmt* return_stmt = dynamic_cast<const ReturnStmt*>(program[0].get());
	ASSERT_NE(return_stmt, nullptr);
	EXPECT_EQ(return_stmt->getValue(), nullptr);
}

TEST_F(ParserTestFixture, ReturnStmtMissingSemicolonFailed) {
	// return 1
	expectParseThrows({
		{TokenType::RETURN, "return", "return", 1},
		{TokenType::NUMBER, "1", 1.0, 1},
		{TokenType::END_OF_FILE, "\n", "\n", 1}
	});
}

TEST_F(ParserTestFixture, CallExprNoArgumentsPassed) {
	// noop();
	const auto& program = buildAndParseProgram({
		{TokenType::IDENTIFIER, "noop", "noop", 1},
		{TokenType::LEFT_PAREN, "(", "(", 1},
		{TokenType::RIGHT_PAREN, ")", ")", 1},
		{TokenType::SEMICOLON, ";", ";", 1},
		{TokenType::END_OF_FILE, "\n", "\n", 1}
	});

	ASSERT_EQ(program.size(), 1);
	const ExpressionStmt* expr_stmt = dynamic_cast<const ExpressionStmt*>(program[0].get());
	ASSERT_NE(expr_stmt, nullptr);
	const CallExpr* call = dynamic_cast<const CallExpr*>(expr_stmt->getExpr());
	ASSERT_NE(call, nullptr);
	ASSERT_NO_FATAL_FAILURE(expectVariable(call->getCallee(), "noop"));
	EXPECT_TRUE(call->getArguments().empty());
}

TEST_F(ParserTestFixture, CallExprWithArgumentsPassed) {
	// add(1, 2);
	const auto& program = buildAndParseProgram({
		{TokenType::IDENTIFIER, "add", "add", 1},
		{TokenType::LEFT_PAREN, "(", "(", 1},
		{TokenType::NUMBER, "1", 1.0, 1},
		{TokenType::COMMA, ",", ",", 1},
		{TokenType::NUMBER, "2", 2.0, 1},
		{TokenType::RIGHT_PAREN, ")", ")", 1},
		{TokenType::SEMICOLON, ";", ";", 1},
		{TokenType::END_OF_FILE, "\n", "\n", 1}
	});

	ASSERT_EQ(program.size(), 1);
	const ExpressionStmt* expr_stmt = dynamic_cast<const ExpressionStmt*>(program[0].get());
	ASSERT_NE(expr_stmt, nullptr);
	const CallExpr* call = dynamic_cast<const CallExpr*>(expr_stmt->getExpr());
	ASSERT_NE(call, nullptr);
	ASSERT_EQ(call->getArguments().size(), 2u);
	ASSERT_NO_FATAL_FAILURE(expectLiteral(call->getArguments()[0].get(), TokenType::NUMBER, "1"));
	ASSERT_NO_FATAL_FAILURE(expectLiteral(call->getArguments()[1].get(), TokenType::NUMBER, "2"));
}

TEST_F(ParserTestFixture, CallExprChainedCallPassed) {
	// fact(n)(); — 호출 결과에 대해서도 연쇄 호출이 파싱되어야 한다.
	const auto& program = buildAndParseProgram({
		{TokenType::IDENTIFIER, "fact", "fact", 1},
		{TokenType::LEFT_PAREN, "(", "(", 1},
		{TokenType::IDENTIFIER, "n", "n", 1},
		{TokenType::RIGHT_PAREN, ")", ")", 1},
		{TokenType::LEFT_PAREN, "(", "(", 1},
		{TokenType::RIGHT_PAREN, ")", ")", 1},
		{TokenType::SEMICOLON, ";", ";", 1},
		{TokenType::END_OF_FILE, "\n", "\n", 1}
	});

	ASSERT_EQ(program.size(), 1);
	const ExpressionStmt* expr_stmt = dynamic_cast<const ExpressionStmt*>(program[0].get());
	ASSERT_NE(expr_stmt, nullptr);
	const CallExpr* outer_call = dynamic_cast<const CallExpr*>(expr_stmt->getExpr());
	ASSERT_NE(outer_call, nullptr);
	EXPECT_TRUE(outer_call->getArguments().empty());
	EXPECT_NE(dynamic_cast<const CallExpr*>(outer_call->getCallee()), nullptr);
}

TEST_F(ParserTestFixture, CallExprMissingClosingParenFailed) {
	// add(1, 2;
	expectParseThrows({
		{TokenType::IDENTIFIER, "add", "add", 1},
		{TokenType::LEFT_PAREN, "(", "(", 1},
		{TokenType::NUMBER, "1", 1.0, 1},
		{TokenType::COMMA, ",", ",", 1},
		{TokenType::NUMBER, "2", 2.0, 1},
		{TokenType::SEMICOLON, ";", ";", 1},
		{TokenType::END_OF_FILE, "\n", "\n", 1}
	});
}

TEST_F(ParserTestFixture, RecursiveFunctionDeclarationPassed) {
	// Func fact(n) { if (n <= 1) return 1; return n * fact(n - 1); }
	const auto& program = buildAndParseProgram({
		{TokenType::FUNC, "Func", "Func", 1},
		{TokenType::IDENTIFIER, "fact", "fact", 1},
		{TokenType::LEFT_PAREN, "(", "(", 1},
		{TokenType::IDENTIFIER, "n", "n", 1},
		{TokenType::RIGHT_PAREN, ")", ")", 1},
		{TokenType::LEFT_BRACE, "{", "{", 1},
		{TokenType::IF, "if", "if", 1},
		{TokenType::LEFT_PAREN, "(", "(", 1},
		{TokenType::IDENTIFIER, "n", "n", 1},
		{TokenType::LESS_EQUAL, "<=", "<=", 1},
		{TokenType::NUMBER, "1", 1.0, 1},
		{TokenType::RIGHT_PAREN, ")", ")", 1},
		{TokenType::RETURN, "return", "return", 1},
		{TokenType::NUMBER, "1", 1.0, 1},
		{TokenType::SEMICOLON, ";", ";", 1},
		{TokenType::RETURN, "return", "return", 1},
		{TokenType::IDENTIFIER, "n", "n", 1},
		{TokenType::STAR, "*", "*", 1},
		{TokenType::IDENTIFIER, "fact", "fact", 1},
		{TokenType::LEFT_PAREN, "(", "(", 1},
		{TokenType::IDENTIFIER, "n", "n", 1},
		{TokenType::MINUS, "-", "-", 1},
		{TokenType::NUMBER, "1", 1.0, 1},
		{TokenType::RIGHT_PAREN, ")", ")", 1},
		{TokenType::SEMICOLON, ";", ";", 1},
		{TokenType::RIGHT_BRACE, "}", "}", 1},
		{TokenType::END_OF_FILE, "\n", "\n", 1}
	});

	ASSERT_EQ(program.size(), 1);
	const FunctionStmt* function_stmt = dynamic_cast<const FunctionStmt*>(program[0].get());
	ASSERT_NE(function_stmt, nullptr);
	ASSERT_EQ(function_stmt->getBody()->getStatements().size(), 2u);
}

TEST_F(ParserTestFixture, GetExprPassed) {
	// print r.speed;
	const auto& program = buildAndParseProgram({
		{TokenType::PRINT, "print", "print", 1},
		{TokenType::IDENTIFIER, "r", "r", 1},
		{TokenType::DOT, ".", ".", 1},
		{TokenType::IDENTIFIER, "speed", "speed", 1},
		{TokenType::SEMICOLON, ";", ";", 1},
		{TokenType::END_OF_FILE, "\n", "\n", 1}
	});

	ASSERT_EQ(program.size(), 1);
	const PrintStmt* print_stmt = dynamic_cast<const PrintStmt*>(program[0].get());
	ASSERT_NE(print_stmt, nullptr);
	const GetExpr* get = dynamic_cast<const GetExpr*>(print_stmt->getExpr());
	ASSERT_NE(get, nullptr);
	EXPECT_EQ(get->getName().getLexeme(), "speed");
}

TEST_F(ParserTestFixture, SetExprPassed) {
	// r.speed = 10;
	const auto& program = buildAndParseProgram({
		{TokenType::IDENTIFIER, "r", "r", 1},
		{TokenType::DOT, ".", ".", 1},
		{TokenType::IDENTIFIER, "speed", "speed", 1},
		{TokenType::EQUAL, "=", "=", 1},
		{TokenType::NUMBER, "10", 10.0, 1},
		{TokenType::SEMICOLON, ";", ";", 1},
		{TokenType::END_OF_FILE, "\n", "\n", 1}
	});

	ASSERT_EQ(program.size(), 1);
	const ExpressionStmt* expr_stmt = dynamic_cast<const ExpressionStmt*>(program[0].get());
	ASSERT_NE(expr_stmt, nullptr);
	const SetExpr* set = dynamic_cast<const SetExpr*>(expr_stmt->getExpr());
	ASSERT_NE(set, nullptr);
	EXPECT_EQ(set->getName().getLexeme(), "speed");
}

TEST_F(ParserTestFixture, ClassStmtWithSuperclassAndMethodsPassed) {
	// Class SpeedRobot : Robot { move(dist) { } }
	const auto& program = buildAndParseProgram({
		{TokenType::CLASS, "Class", "Class", 1},
		{TokenType::IDENTIFIER, "SpeedRobot", "SpeedRobot", 1},
		{TokenType::COLON, ":", ":", 1},
		{TokenType::IDENTIFIER, "Robot", "Robot", 1},
		{TokenType::LEFT_BRACE, "{", "{", 1},
		{TokenType::IDENTIFIER, "move", "move", 1},
		{TokenType::LEFT_PAREN, "(", "(", 1},
		{TokenType::IDENTIFIER, "dist", "dist", 1},
		{TokenType::RIGHT_PAREN, ")", ")", 1},
		{TokenType::LEFT_BRACE, "{", "{", 1},
		{TokenType::RIGHT_BRACE, "}", "}", 1},
		{TokenType::RIGHT_BRACE, "}", "}", 1},
		{TokenType::END_OF_FILE, "\n", "\n", 1}
	});

	ASSERT_EQ(program.size(), 1);
	const ClassStmt* class_stmt = dynamic_cast<const ClassStmt*>(program[0].get());
	ASSERT_NE(class_stmt, nullptr);
	EXPECT_EQ(class_stmt->getName().getLexeme(), "SpeedRobot");
	ASSERT_NE(class_stmt->getSuperclass(), nullptr);
	EXPECT_EQ(class_stmt->getSuperclass()->getToken().getLexeme(), "Robot");
	ASSERT_EQ(class_stmt->getMethods().size(), 1u);
	EXPECT_EQ(class_stmt->getMethods()[0]->getName().getLexeme(), "move");
}

TEST_F(ParserTestFixture, ClassStmtWithoutSuperclassPassed) {
	// Class Robot { }
	const auto& program = buildAndParseProgram({
		{TokenType::CLASS, "Class", "Class", 1},
		{TokenType::IDENTIFIER, "Robot", "Robot", 1},
		{TokenType::LEFT_BRACE, "{", "{", 1},
		{TokenType::RIGHT_BRACE, "}", "}", 1},
		{TokenType::END_OF_FILE, "\n", "\n", 1}
	});

	ASSERT_EQ(program.size(), 1);
	const ClassStmt* class_stmt = dynamic_cast<const ClassStmt*>(program[0].get());
	ASSERT_NE(class_stmt, nullptr);
	EXPECT_EQ(class_stmt->getSuperclass(), nullptr);
	EXPECT_EQ(class_stmt->getMethods().size(), 0u);
}

TEST_F(ParserTestFixture, ThisExprPassed) {
	// print this;
	const auto& program = buildAndParseProgram({
		{TokenType::PRINT, "print", "print", 1},
		{TokenType::THIS, "this", "this", 1},
		{TokenType::SEMICOLON, ";", ";", 1},
		{TokenType::END_OF_FILE, "\n", "\n", 1}
	});

	ASSERT_EQ(program.size(), 1);
	const PrintStmt* print_stmt = dynamic_cast<const PrintStmt*>(program[0].get());
	ASSERT_NE(print_stmt, nullptr);
	EXPECT_NE(dynamic_cast<const ThisExpr*>(print_stmt->getExpr()), nullptr);
}

TEST_F(ParserTestFixture, SuperExprPassed) {
	// Super.move(dist);
	const auto& program = buildAndParseProgram({
		{TokenType::SUPER, "Super", "Super", 1},
		{TokenType::DOT, ".", ".", 1},
		{TokenType::IDENTIFIER, "move", "move", 1},
		{TokenType::LEFT_PAREN, "(", "(", 1},
		{TokenType::IDENTIFIER, "dist", "dist", 1},
		{TokenType::RIGHT_PAREN, ")", ")", 1},
		{TokenType::SEMICOLON, ";", ";", 1},
		{TokenType::END_OF_FILE, "\n", "\n", 1}
	});

	ASSERT_EQ(program.size(), 1);
	const ExpressionStmt* expr_stmt = dynamic_cast<const ExpressionStmt*>(program[0].get());
	ASSERT_NE(expr_stmt, nullptr);
	const CallExpr* call = dynamic_cast<const CallExpr*>(expr_stmt->getExpr());
	ASSERT_NE(call, nullptr);
	const SuperExpr* super_expr = dynamic_cast<const SuperExpr*>(call->getCallee());
	ASSERT_NE(super_expr, nullptr);
	EXPECT_EQ(super_expr->getMethod().getLexeme(), "move");
}

TEST_F(ParserTestFixture, InstanceOfExprPassed) {
	// print (w instanceof Robot);
	const auto& program = buildAndParseProgram({
		{TokenType::PRINT, "print", "print", 1},
		{TokenType::LEFT_PAREN, "(", "(", 1},
		{TokenType::IDENTIFIER, "w", "w", 1},
		{TokenType::INSTANCEOF, "instanceof", "instanceof", 1},
		{TokenType::IDENTIFIER, "Robot", "Robot", 1},
		{TokenType::RIGHT_PAREN, ")", ")", 1},
		{TokenType::SEMICOLON, ";", ";", 1},
		{TokenType::END_OF_FILE, "\n", "\n", 1}
	});

	ASSERT_EQ(program.size(), 1);
	const PrintStmt* print_stmt = dynamic_cast<const PrintStmt*>(program[0].get());
	ASSERT_NE(print_stmt, nullptr);
	const GroupingExpr* grouping = dynamic_cast<const GroupingExpr*>(print_stmt->getExpr());
	ASSERT_NE(grouping, nullptr);
	const InstanceOfExpr* instance_of = dynamic_cast<const InstanceOfExpr*>(grouping->getExpr());
	ASSERT_NE(instance_of, nullptr);
	EXPECT_EQ(instance_of->getClassName().getLexeme(), "Robot");
}
TEST_F(ParserTestFixture, ArrayExprWithLiteralSizePassed) {
	// Array(3);
	const auto& program = buildAndParseProgram({
		{TokenType::ARRAY, "Array", "Array", 1},
		{TokenType::LEFT_PAREN, "(", "(", 1},
		{TokenType::NUMBER, "3", 3.0, 1},
		{TokenType::RIGHT_PAREN, ")", ")", 1},
		{TokenType::SEMICOLON, ";", ";", 1},
		{TokenType::END_OF_FILE, "\n", "\n", 1}
	});

	ASSERT_EQ(program.size(), 1);
	const ExpressionStmt* expr_stmt = dynamic_cast<const ExpressionStmt*>(program[0].get());
	ASSERT_NE(expr_stmt, nullptr);
	const ArrayExpr* array_expr = dynamic_cast<const ArrayExpr*>(expr_stmt->getExpr());
	ASSERT_NE(array_expr, nullptr);
	ASSERT_NO_FATAL_FAILURE(expectLiteral(array_expr->getSize(), TokenType::NUMBER, "3"));
}

TEST_F(ParserTestFixture, ArrayExprWithVariableSizePassed) {
	// Array(n);
	const auto& program = buildAndParseProgram({
		{TokenType::ARRAY, "Array", "Array", 1},
		{TokenType::LEFT_PAREN, "(", "(", 1},
		{TokenType::IDENTIFIER, "n", "n", 1},
		{TokenType::RIGHT_PAREN, ")", ")", 1},
		{TokenType::SEMICOLON, ";", ";", 1},
		{TokenType::END_OF_FILE, "\n", "\n", 1}
	});

	ASSERT_EQ(program.size(), 1);
	const ExpressionStmt* expr_stmt = dynamic_cast<const ExpressionStmt*>(program[0].get());
	ASSERT_NE(expr_stmt, nullptr);
	const ArrayExpr* array_expr = dynamic_cast<const ArrayExpr*>(expr_stmt->getExpr());
	ASSERT_NE(array_expr, nullptr);
	ASSERT_NO_FATAL_FAILURE(expectVariable(array_expr->getSize(), "n"));
}

TEST_F(ParserTestFixture, ArrayExprLiteralSizeMisuseFailed) {
	// Array("hi");
	expectParseThrows({
		{TokenType::ARRAY, "Array", "Array", 1},
		{TokenType::LEFT_PAREN, "(", "(", 1},
		{TokenType::STRING, "\"hi\"", "hi", 1},
		{TokenType::RIGHT_PAREN, ")", ")", 1},
		{TokenType::SEMICOLON, ";", ";", 1},
		{TokenType::END_OF_FILE, "\n", "\n", 1}
	});
}

TEST_F(ParserTestFixture, ArrayExprMissingParenFailed) {
	// Array 3);
	expectParseThrows({
		{TokenType::ARRAY, "Array", "Array", 1},
		{TokenType::NUMBER, "3", 3.0, 1},
		{TokenType::RIGHT_PAREN, ")", ")", 1},
		{TokenType::SEMICOLON, ";", ";", 1},
		{TokenType::END_OF_FILE, "\n", "\n", 1}
	});
}

TEST_F(ParserTestFixture, IndexExprReadWithLiteralIndexPassed) {
	// arr[0];
	const auto& program = buildAndParseProgram({
		{TokenType::IDENTIFIER, "arr", "arr", 1},
		{TokenType::LEFT_BRACKET, "[", "[", 1},
		{TokenType::NUMBER, "0", 0.0, 1},
		{TokenType::RIGHT_BRACKET, "]", "]", 1},
		{TokenType::SEMICOLON, ";", ";", 1},
		{TokenType::END_OF_FILE, "\n", "\n", 1}
	});

	ASSERT_EQ(program.size(), 1);
	const ExpressionStmt* expr_stmt = dynamic_cast<const ExpressionStmt*>(program[0].get());
	ASSERT_NE(expr_stmt, nullptr);
	const IndexExpr* index_expr = dynamic_cast<const IndexExpr*>(expr_stmt->getExpr());
	ASSERT_NE(index_expr, nullptr);
	ASSERT_NO_FATAL_FAILURE(expectVariable(index_expr->getArray(), "arr"));
	ASSERT_NO_FATAL_FAILURE(expectLiteral(index_expr->getIndex(), TokenType::NUMBER, "0"));
}

TEST_F(ParserTestFixture, IndexExprReadWithComputedIndexPassed) {
	// arr[i - 1];
	const auto& program = buildAndParseProgram({
		{TokenType::IDENTIFIER, "arr", "arr", 1},
		{TokenType::LEFT_BRACKET, "[", "[", 1},
		{TokenType::IDENTIFIER, "i", "i", 1},
		{TokenType::MINUS, "-", "-", 1},
		{TokenType::NUMBER, "1", 1.0, 1},
		{TokenType::RIGHT_BRACKET, "]", "]", 1},
		{TokenType::SEMICOLON, ";", ";", 1},
		{TokenType::END_OF_FILE, "\n", "\n", 1}
	});

	ASSERT_EQ(program.size(), 1);
	const ExpressionStmt* expr_stmt = dynamic_cast<const ExpressionStmt*>(program[0].get());
	ASSERT_NE(expr_stmt, nullptr);
	const IndexExpr* index_expr = dynamic_cast<const IndexExpr*>(expr_stmt->getExpr());
	ASSERT_NE(index_expr, nullptr);
	ASSERT_NO_FATAL_FAILURE(expectBinary(index_expr->getIndex(), TokenType::MINUS, "-"));
}

TEST_F(ParserTestFixture, IndexExprLiteralIndexMisuseFailed) {
	// arr["hi"];
	expectParseThrows({
		{TokenType::IDENTIFIER, "arr", "arr", 1},
		{TokenType::LEFT_BRACKET, "[", "[", 1},
		{TokenType::STRING, "\"hi\"", "hi", 1},
		{TokenType::RIGHT_BRACKET, "]", "]", 1},
		{TokenType::SEMICOLON, ";", ";", 1},
		{TokenType::END_OF_FILE, "\n", "\n", 1}
	});
}

TEST_F(ParserTestFixture, IndexExprMissingClosingBracketFailed) {
	// arr[0;
	expectParseThrows({
		{TokenType::IDENTIFIER, "arr", "arr", 1},
		{TokenType::LEFT_BRACKET, "[", "[", 1},
		{TokenType::NUMBER, "0", 0.0, 1},
		{TokenType::SEMICOLON, ";", ";", 1},
		{TokenType::END_OF_FILE, "\n", "\n", 1}
	});
}

TEST_F(ParserTestFixture, ChainedIndexExprPassed) {
	// arr[0][1];
	const auto& program = buildAndParseProgram({
		{TokenType::IDENTIFIER, "arr", "arr", 1},
		{TokenType::LEFT_BRACKET, "[", "[", 1},
		{TokenType::NUMBER, "0", 0.0, 1},
		{TokenType::RIGHT_BRACKET, "]", "]", 1},
		{TokenType::LEFT_BRACKET, "[", "[", 1},
		{TokenType::NUMBER, "1", 1.0, 1},
		{TokenType::RIGHT_BRACKET, "]", "]", 1},
		{TokenType::SEMICOLON, ";", ";", 1},
		{TokenType::END_OF_FILE, "\n", "\n", 1}
	});

	ASSERT_EQ(program.size(), 1);
	const ExpressionStmt* expr_stmt = dynamic_cast<const ExpressionStmt*>(program[0].get());
	ASSERT_NE(expr_stmt, nullptr);
	const IndexExpr* outer_index = dynamic_cast<const IndexExpr*>(expr_stmt->getExpr());
	ASSERT_NE(outer_index, nullptr);
	ASSERT_NO_FATAL_FAILURE(expectLiteral(outer_index->getIndex(), TokenType::NUMBER, "1"));
	const IndexExpr* inner_index = dynamic_cast<const IndexExpr*>(outer_index->getArray());
	ASSERT_NE(inner_index, nullptr);
	ASSERT_NO_FATAL_FAILURE(expectVariable(inner_index->getArray(), "arr"));
	ASSERT_NO_FATAL_FAILURE(expectLiteral(inner_index->getIndex(), TokenType::NUMBER, "0"));
}

TEST_F(ParserTestFixture, ArrayOfCallResultIndexingPassed) {
	// foo()[0];
	const auto& program = buildAndParseProgram({
		{TokenType::IDENTIFIER, "foo", "foo", 1},
		{TokenType::LEFT_PAREN, "(", "(", 1},
		{TokenType::RIGHT_PAREN, ")", ")", 1},
		{TokenType::LEFT_BRACKET, "[", "[", 1},
		{TokenType::NUMBER, "0", 0.0, 1},
		{TokenType::RIGHT_BRACKET, "]", "]", 1},
		{TokenType::SEMICOLON, ";", ";", 1},
		{TokenType::END_OF_FILE, "\n", "\n", 1}
	});

	ASSERT_EQ(program.size(), 1);
	const ExpressionStmt* expr_stmt = dynamic_cast<const ExpressionStmt*>(program[0].get());
	ASSERT_NE(expr_stmt, nullptr);
	const IndexExpr* index_expr = dynamic_cast<const IndexExpr*>(expr_stmt->getExpr());
	ASSERT_NE(index_expr, nullptr);
	EXPECT_NE(dynamic_cast<const CallExpr*>(index_expr->getArray()), nullptr);
}

TEST_F(ParserTestFixture, IndexSetExprWithLiteralIndexPassed) {
	// arr[0] = 7;
	const auto& program = buildAndParseProgram({
		{TokenType::IDENTIFIER, "arr", "arr", 1},
		{TokenType::LEFT_BRACKET, "[", "[", 1},
		{TokenType::NUMBER, "0", 0.0, 1},
		{TokenType::RIGHT_BRACKET, "]", "]", 1},
		{TokenType::EQUAL, "=", "=", 1},
		{TokenType::NUMBER, "7", 7.0, 1},
		{TokenType::SEMICOLON, ";", ";", 1},
		{TokenType::END_OF_FILE, "\n", "\n", 1}
	});

	ASSERT_EQ(program.size(), 1);
	const ExpressionStmt* expr_stmt = dynamic_cast<const ExpressionStmt*>(program[0].get());
	ASSERT_NE(expr_stmt, nullptr);
	const IndexSetExpr* index_set = dynamic_cast<const IndexSetExpr*>(expr_stmt->getExpr());
	ASSERT_NE(index_set, nullptr);
	ASSERT_NO_FATAL_FAILURE(expectVariable(index_set->getArray(), "arr"));
	ASSERT_NO_FATAL_FAILURE(expectLiteral(index_set->getIndex(), TokenType::NUMBER, "0"));
	ASSERT_NO_FATAL_FAILURE(expectLiteral(index_set->getValue(), TokenType::NUMBER, "7"));
}

TEST_F(ParserTestFixture, IndexSetExprWithComputedIndexPassed) {
	// arr[i - 1] = 7;
	const auto& program = buildAndParseProgram({
		{TokenType::IDENTIFIER, "arr", "arr", 1},
		{TokenType::LEFT_BRACKET, "[", "[", 1},
		{TokenType::IDENTIFIER, "i", "i", 1},
		{TokenType::MINUS, "-", "-", 1},
		{TokenType::NUMBER, "1", 1.0, 1},
		{TokenType::RIGHT_BRACKET, "]", "]", 1},
		{TokenType::EQUAL, "=", "=", 1},
		{TokenType::NUMBER, "7", 7.0, 1},
		{TokenType::SEMICOLON, ";", ";", 1},
		{TokenType::END_OF_FILE, "\n", "\n", 1}
	});

	ASSERT_EQ(program.size(), 1);
	const ExpressionStmt* expr_stmt = dynamic_cast<const ExpressionStmt*>(program[0].get());
	ASSERT_NE(expr_stmt, nullptr);
	const IndexSetExpr* index_set = dynamic_cast<const IndexSetExpr*>(expr_stmt->getExpr());
	ASSERT_NE(index_set, nullptr);
	ASSERT_NO_FATAL_FAILURE(expectBinary(index_set->getIndex(), TokenType::MINUS, "-"));
}

TEST_F(ParserTestFixture, IndexSetExprMissingValueFailed) {
	// arr[0] = ;
	expectParseThrows({
		{TokenType::IDENTIFIER, "arr", "arr", 1},
		{TokenType::LEFT_BRACKET, "[", "[", 1},
		{TokenType::NUMBER, "0", 0.0, 1},
		{TokenType::RIGHT_BRACKET, "]", "]", 1},
		{TokenType::EQUAL, "=", "=", 1},
		{TokenType::SEMICOLON, ";", ";", 1},
		{TokenType::END_OF_FILE, "\n", "\n", 1}
	});
}

TEST_F(ParserTestFixture, ImportStmtWithAliasPassed) {
	// import "a.txt" alias a;
	const auto& program = buildAndParseProgram({
		{TokenType::IMPORT, "import", "import", 1},
		{TokenType::STRING, "\"a.txt\"", "a.txt", 1},
		{TokenType::ALIAS, "alias", "alias", 1},
		{TokenType::IDENTIFIER, "a", "a", 1},
		{TokenType::SEMICOLON, ";", ";", 1},
		{TokenType::END_OF_FILE, "\n", "\n", 1}
	});

	ASSERT_EQ(program.size(), 1);
	const ImportStmt* import_stmt = dynamic_cast<const ImportStmt*>(program[0].get());
	ASSERT_NE(import_stmt, nullptr);
	EXPECT_EQ(std::get<string>(import_stmt->getPath().getLiteral()), "a.txt");
	EXPECT_EQ(import_stmt->getAlias().getLexeme(), "a");
}

TEST_F(ParserTestFixture, ImportStmtMissingAliasKeywordFailed) {
	// import "a.txt" a;
	expectParseThrows({
		{TokenType::IMPORT, "import", "import", 1},
		{TokenType::STRING, "\"a.txt\"", "a.txt", 1},
		{TokenType::IDENTIFIER, "a", "a", 1},
		{TokenType::SEMICOLON, ";", ";", 1},
		{TokenType::END_OF_FILE, "\n", "\n", 1}
	});
}

TEST_F(ParserTestFixture, ImportStmtMissingPathStringFailed) {
	// import alias a;
	expectParseThrows({
		{TokenType::IMPORT, "import", "import", 1},
		{TokenType::ALIAS, "alias", "alias", 1},
		{TokenType::IDENTIFIER, "a", "a", 1},
		{TokenType::SEMICOLON, ";", ";", 1},
		{TokenType::END_OF_FILE, "\n", "\n", 1}
	});
}
