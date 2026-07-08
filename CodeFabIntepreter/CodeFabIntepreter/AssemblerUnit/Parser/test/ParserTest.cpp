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

	const Expression* expectArrayAndGetSize(const Expression* expr) {
		const ArrayExpr* array_expr = dynamic_cast<const ArrayExpr*>(expr);

		EXPECT_NE(array_expr, nullptr);
		if (array_expr == nullptr) return nullptr;

		return array_expr->getSize();
	}

	const IndexExpr* expectIndex(const Expression* expr) {
		const IndexExpr* index_expr = dynamic_cast<const IndexExpr*>(expr);

		EXPECT_NE(index_expr, nullptr);
		return index_expr;
	}

	const IndexSetExpr* expectIndexSet(const Expression* expr) {
		const IndexSetExpr* index_set_expr = dynamic_cast<const IndexSetExpr*>(expr);

		EXPECT_NE(index_set_expr, nullptr);
		return index_set_expr;
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

TEST_F(ParserTestFixture, VarDeclareStmtArraySizeNumberLiteralPassed) {
	// var arr = Array(3);
	VarDeclareStmt* stmt = buildAndParseVarDeclareStmt({
		{TokenType::ARRAY, "Array", "Array", 1},
		{TokenType::LEFT_PAREN, "(", "(", 1},
		{TokenType::NUMBER, "3", 3.0, 1},
		{TokenType::RIGHT_PAREN, ")", ")", 1}
	});

	ASSERT_NO_FATAL_FAILURE(expectDeclaredName(stmt, "a"));
	const Expression* size = expectArrayAndGetSize(stmt->getInitializer());
	ASSERT_NO_FATAL_FAILURE(expectLiteral(size, TokenType::NUMBER, "3"));
}

TEST_F(ParserTestFixture, VarDeclareStmtArraySizeVariablePassed) {
	// var arr = Array(n);
	VarDeclareStmt* stmt = buildAndParseVarDeclareStmt({
		{TokenType::ARRAY, "Array", "Array", 1},
		{TokenType::LEFT_PAREN, "(", "(", 1},
		{TokenType::IDENTIFIER, "n", "n", 1},
		{TokenType::RIGHT_PAREN, ")", ")", 1}
	});

	ASSERT_NO_FATAL_FAILURE(expectDeclaredName(stmt, "a"));
	const Expression* size = expectArrayAndGetSize(stmt->getInitializer());
	ASSERT_NO_FATAL_FAILURE(expectVariable(size, "n"));
}

TEST_F(ParserTestFixture, VarDeclareStmtArraySizeStringLiteralFailed) {
	// var a = Array("three");
	expectParseThrows({
		{TokenType::VAR, "var", "var", 1},
		{TokenType::IDENTIFIER, "a", "a", 1},
		{TokenType::EQUAL, "=", "=", 1},
		{TokenType::ARRAY, "Array", "Array", 1},
		{TokenType::LEFT_PAREN, "(", "(", 1},
		{TokenType::STRING, "\"three\"", "three", 1},
		{TokenType::RIGHT_PAREN, ")", ")", 1},
		{TokenType::SEMICOLON, ";", ";", 1},
		{TokenType::END_OF_FILE, "\n", "\n", 1}
	});
}

TEST_F(ParserTestFixture, VarDeclareStmtArraySizeBooleanLiteralFailed) {
	// var a = Array(true);
	expectParseThrows({
		{TokenType::VAR, "var", "var", 1},
		{TokenType::IDENTIFIER, "a", "a", 1},
		{TokenType::EQUAL, "=", "=", 1},
		{TokenType::ARRAY, "Array", "Array", 1},
		{TokenType::LEFT_PAREN, "(", "(", 1},
		{TokenType::TRUE, "true", true, 1},
		{TokenType::RIGHT_PAREN, ")", ")", 1},
		{TokenType::SEMICOLON, ";", ";", 1},
		{TokenType::END_OF_FILE, "\n", "\n", 1}
	});
}

TEST_F(ParserTestFixture, VarDeclareStmtArrayMissingLeftParenFailed) {
	// var a = Array 3);
	expectParseThrows({
		{TokenType::VAR, "var", "var", 1},
		{TokenType::IDENTIFIER, "a", "a", 1},
		{TokenType::EQUAL, "=", "=", 1},
		{TokenType::ARRAY, "Array", "Array", 1},
		{TokenType::NUMBER, "3", 3.0, 1},
		{TokenType::RIGHT_PAREN, ")", ")", 1},
		{TokenType::SEMICOLON, ";", ";", 1},
		{TokenType::END_OF_FILE, "\n", "\n", 1}
	});
}

TEST_F(ParserTestFixture, VarDeclareStmtArrayMissingRightParenFailed) {
	// var a = Array(3;
	expectParseThrows({
		{TokenType::VAR, "var", "var", 1},
		{TokenType::IDENTIFIER, "a", "a", 1},
		{TokenType::EQUAL, "=", "=", 1},
		{TokenType::ARRAY, "Array", "Array", 1},
		{TokenType::LEFT_PAREN, "(", "(", 1},
		{TokenType::NUMBER, "3", 3.0, 1},
		{TokenType::SEMICOLON, ";", ";", 1},
		{TokenType::END_OF_FILE, "\n", "\n", 1}
	});
}

TEST_F(ParserTestFixture, PrintStmtIndexExprNumberLiteralPassed) {
	// print arr[0];
	const auto& program = buildAndParseProgram({
		{TokenType::PRINT, "print", "print", 1},
		{TokenType::IDENTIFIER, "arr", "arr", 1},
		{TokenType::LEFT_BRACKET, "[", "[", 1},
		{TokenType::NUMBER, "0", 0.0, 1},
		{TokenType::RIGHT_BRACKET, "]", "]", 1},
		{TokenType::SEMICOLON, ";", ";", 1},
		{TokenType::END_OF_FILE, "\n", "\n", 1}
	});

	ASSERT_EQ(program.size(), 1);
	const PrintStmt* print_stmt = dynamic_cast<const PrintStmt*>(program[0].get());
	ASSERT_NE(print_stmt, nullptr);

	const IndexExpr* index_expr = expectIndex(print_stmt->getExpr());
	ASSERT_NE(index_expr, nullptr);
	ASSERT_NO_FATAL_FAILURE(expectVariable(index_expr->getArray(), "arr"));
	ASSERT_NO_FATAL_FAILURE(expectLiteral(index_expr->getIndex(), TokenType::NUMBER, "0"));
}

TEST_F(ParserTestFixture, PrintStmtIndexExprBinaryIndexPassed) {
	// print arr[i - 2];
	const auto& program = buildAndParseProgram({
		{TokenType::PRINT, "print", "print", 1},
		{TokenType::IDENTIFIER, "arr", "arr", 1},
		{TokenType::LEFT_BRACKET, "[", "[", 1},
		{TokenType::IDENTIFIER, "i", "i", 1},
		{TokenType::MINUS, "-", "-", 1},
		{TokenType::NUMBER, "2", 2.0, 1},
		{TokenType::RIGHT_BRACKET, "]", "]", 1},
		{TokenType::SEMICOLON, ";", ";", 1},
		{TokenType::END_OF_FILE, "\n", "\n", 1}
	});

	ASSERT_EQ(program.size(), 1);
	const PrintStmt* print_stmt = dynamic_cast<const PrintStmt*>(program[0].get());
	ASSERT_NE(print_stmt, nullptr);

	const IndexExpr* index_expr = expectIndex(print_stmt->getExpr());
	ASSERT_NE(index_expr, nullptr);
	ASSERT_NO_FATAL_FAILURE(expectVariable(index_expr->getArray(), "arr"));
	ASSERT_NO_FATAL_FAILURE(expectBinary(index_expr->getIndex(), TokenType::MINUS, "-"));
}

TEST_F(ParserTestFixture, IndexExprChainedPassed) {
	// print arr[0][1];
	const auto& program = buildAndParseProgram({
		{TokenType::PRINT, "print", "print", 1},
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
	const PrintStmt* print_stmt = dynamic_cast<const PrintStmt*>(program[0].get());
	ASSERT_NE(print_stmt, nullptr);

	const IndexExpr* outer = expectIndex(print_stmt->getExpr());
	ASSERT_NE(outer, nullptr);
	ASSERT_NO_FATAL_FAILURE(expectLiteral(outer->getIndex(), TokenType::NUMBER, "1"));

	const IndexExpr* inner = expectIndex(outer->getArray());
	ASSERT_NE(inner, nullptr);
	ASSERT_NO_FATAL_FAILURE(expectVariable(inner->getArray(), "arr"));
	ASSERT_NO_FATAL_FAILURE(expectLiteral(inner->getIndex(), TokenType::NUMBER, "0"));
}

TEST_F(ParserTestFixture, IndexExprOfArrayCreationPassed) {
	// print Array(3)[0];
	const auto& program = buildAndParseProgram({
		{TokenType::PRINT, "print", "print", 1},
		{TokenType::ARRAY, "Array", "Array", 1},
		{TokenType::LEFT_PAREN, "(", "(", 1},
		{TokenType::NUMBER, "3", 3.0, 1},
		{TokenType::RIGHT_PAREN, ")", ")", 1},
		{TokenType::LEFT_BRACKET, "[", "[", 1},
		{TokenType::NUMBER, "0", 0.0, 1},
		{TokenType::RIGHT_BRACKET, "]", "]", 1},
		{TokenType::SEMICOLON, ";", ";", 1},
		{TokenType::END_OF_FILE, "\n", "\n", 1}
	});

	ASSERT_EQ(program.size(), 1);
	const PrintStmt* print_stmt = dynamic_cast<const PrintStmt*>(program[0].get());
	ASSERT_NE(print_stmt, nullptr);

	const IndexExpr* index_expr = expectIndex(print_stmt->getExpr());
	ASSERT_NE(index_expr, nullptr);
	ASSERT_NO_FATAL_FAILURE(expectLiteral(index_expr->getIndex(), TokenType::NUMBER, "0"));

	const Expression* size = expectArrayAndGetSize(index_expr->getArray());
	ASSERT_NO_FATAL_FAILURE(expectLiteral(size, TokenType::NUMBER, "3"));
}

TEST_F(ParserTestFixture, IndexExprStringLiteralFailed) {
	// print arr["x"];
	expectParseThrows({
		{TokenType::PRINT, "print", "print", 1},
		{TokenType::IDENTIFIER, "arr", "arr", 1},
		{TokenType::LEFT_BRACKET, "[", "[", 1},
		{TokenType::STRING, "\"x\"", "x", 1},
		{TokenType::RIGHT_BRACKET, "]", "]", 1},
		{TokenType::SEMICOLON, ";", ";", 1},
		{TokenType::END_OF_FILE, "\n", "\n", 1}
	});
}

TEST_F(ParserTestFixture, IndexExprBooleanLiteralFailed) {
	// print arr[true];
	expectParseThrows({
		{TokenType::PRINT, "print", "print", 1},
		{TokenType::IDENTIFIER, "arr", "arr", 1},
		{TokenType::LEFT_BRACKET, "[", "[", 1},
		{TokenType::TRUE, "true", true, 1},
		{TokenType::RIGHT_BRACKET, "]", "]", 1},
		{TokenType::SEMICOLON, ";", ";", 1},
		{TokenType::END_OF_FILE, "\n", "\n", 1}
	});
}

TEST_F(ParserTestFixture, IndexExprMissingClosingBracketFailed) {
	// print arr[0;
	expectParseThrows({
		{TokenType::PRINT, "print", "print", 1},
		{TokenType::IDENTIFIER, "arr", "arr", 1},
		{TokenType::LEFT_BRACKET, "[", "[", 1},
		{TokenType::NUMBER, "0", 0.0, 1},
		{TokenType::SEMICOLON, ";", ";", 1},
		{TokenType::END_OF_FILE, "\n", "\n", 1}
	});
}

TEST_F(ParserTestFixture, IndexExprEmptyBracketsFailed) {
	// print arr[];
	expectParseThrows({
		{TokenType::PRINT, "print", "print", 1},
		{TokenType::IDENTIFIER, "arr", "arr", 1},
		{TokenType::LEFT_BRACKET, "[", "[", 1},
		{TokenType::RIGHT_BRACKET, "]", "]", 1},
		{TokenType::SEMICOLON, ";", ";", 1},
		{TokenType::END_OF_FILE, "\n", "\n", 1}
	});
}

TEST_F(ParserTestFixture, IndexSetExprNumberLiteralPassed) {
	// arr[0] = 10;
	const auto& program = buildAndParseProgram({
		{TokenType::IDENTIFIER, "arr", "arr", 1},
		{TokenType::LEFT_BRACKET, "[", "[", 1},
		{TokenType::NUMBER, "0", 0.0, 1},
		{TokenType::RIGHT_BRACKET, "]", "]", 1},
		{TokenType::EQUAL, "=", "=", 1},
		{TokenType::NUMBER, "10", 10.0, 1},
		{TokenType::SEMICOLON, ";", ";", 1},
		{TokenType::END_OF_FILE, "\n", "\n", 1}
	});

	ASSERT_EQ(program.size(), 1);
	const ExpressionStmt* expr_stmt = dynamic_cast<const ExpressionStmt*>(program[0].get());
	ASSERT_NE(expr_stmt, nullptr);

	const IndexSetExpr* index_set = expectIndexSet(expr_stmt->getExpr());
	ASSERT_NE(index_set, nullptr);
	ASSERT_NO_FATAL_FAILURE(expectVariable(index_set->getArray(), "arr"));
	ASSERT_NO_FATAL_FAILURE(expectLiteral(index_set->getIndex(), TokenType::NUMBER, "0"));
	ASSERT_NO_FATAL_FAILURE(expectLiteral(index_set->getValue(), TokenType::NUMBER, "10"));
}

TEST_F(ParserTestFixture, IndexSetExprBinaryIndexPassed) {
	// arr[i - 2] = 7;
	const auto& program = buildAndParseProgram({
		{TokenType::IDENTIFIER, "arr", "arr", 1},
		{TokenType::LEFT_BRACKET, "[", "[", 1},
		{TokenType::IDENTIFIER, "i", "i", 1},
		{TokenType::MINUS, "-", "-", 1},
		{TokenType::NUMBER, "2", 2.0, 1},
		{TokenType::RIGHT_BRACKET, "]", "]", 1},
		{TokenType::EQUAL, "=", "=", 1},
		{TokenType::NUMBER, "7", 7.0, 1},
		{TokenType::SEMICOLON, ";", ";", 1},
		{TokenType::END_OF_FILE, "\n", "\n", 1}
	});

	ASSERT_EQ(program.size(), 1);
	const ExpressionStmt* expr_stmt = dynamic_cast<const ExpressionStmt*>(program[0].get());
	ASSERT_NE(expr_stmt, nullptr);

	const IndexSetExpr* index_set = expectIndexSet(expr_stmt->getExpr());
	ASSERT_NE(index_set, nullptr);
	ASSERT_NO_FATAL_FAILURE(expectVariable(index_set->getArray(), "arr"));
	ASSERT_NO_FATAL_FAILURE(expectBinary(index_set->getIndex(), TokenType::MINUS, "-"));
	ASSERT_NO_FATAL_FAILURE(expectLiteral(index_set->getValue(), TokenType::NUMBER, "7"));
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
