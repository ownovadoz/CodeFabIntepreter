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

	const CallExpr* expectCall(const Expression* expr) {
		const CallExpr* call_expr = dynamic_cast<const CallExpr*>(expr);

		EXPECT_NE(call_expr, nullptr);
		return call_expr;
	}

	const GetExpr* expectGet(const Expression* expr) {
		const GetExpr* get_expr = dynamic_cast<const GetExpr*>(expr);

		EXPECT_NE(get_expr, nullptr);
		return get_expr;
	}

	const SetExpr* expectSet(const Expression* expr) {
		const SetExpr* set_expr = dynamic_cast<const SetExpr*>(expr);

		EXPECT_NE(set_expr, nullptr);
		return set_expr;
	}

	void expectThis(const Expression* expr) {
		EXPECT_NE(dynamic_cast<const ThisExpr*>(expr), nullptr);
	}

	const SuperExpr* expectSuper(const Expression* expr) {
		const SuperExpr* super_expr = dynamic_cast<const SuperExpr*>(expr);

		EXPECT_NE(super_expr, nullptr);
		return super_expr;
	}

	const InstanceOfExpr* expectInstanceOf(const Expression* expr) {
		const InstanceOfExpr* instance_of_expr = dynamic_cast<const InstanceOfExpr*>(expr);

		EXPECT_NE(instance_of_expr, nullptr);
		return instance_of_expr;
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

TEST_F(ParserTestFixture, FunctionDeclStmtNoParamsPassed) {
	// Func greet() { return; }
	const auto& program = buildAndParseProgram({
		{TokenType::FUNC, "Func", "Func", 1},
		{TokenType::IDENTIFIER, "greet", "greet", 1},
		{TokenType::LEFT_PAREN, "(", "(", 1},
		{TokenType::RIGHT_PAREN, ")", ")", 1},
		{TokenType::LEFT_BRACE, "{", "{", 1},
		{TokenType::RETURN, "return", "return", 1},
		{TokenType::SEMICOLON, ";", ";", 1},
		{TokenType::RIGHT_BRACE, "}", "}", 1},
		{TokenType::END_OF_FILE, "\n", "\n", 1}
	});

	ASSERT_EQ(program.size(), 1);
	const FunctionDeclStmt* fn = dynamic_cast<const FunctionDeclStmt*>(program[0].get());
	ASSERT_NE(fn, nullptr);
	EXPECT_EQ(fn->getName().getLexeme(), "greet");
	EXPECT_EQ(fn->getParameters().size(), 0u);

	ASSERT_EQ(fn->getBody()->getStatements().size(), 1u);
	const ReturnStmt* return_stmt = dynamic_cast<const ReturnStmt*>(fn->getBody()->getStatements()[0].get());
	ASSERT_NE(return_stmt, nullptr);
	EXPECT_EQ(return_stmt->getValue(), nullptr);
}

TEST_F(ParserTestFixture, FunctionDeclStmtSingleParamPassed) {
	// Func square(x) { return x; }
	const auto& program = buildAndParseProgram({
		{TokenType::FUNC, "Func", "Func", 1},
		{TokenType::IDENTIFIER, "square", "square", 1},
		{TokenType::LEFT_PAREN, "(", "(", 1},
		{TokenType::IDENTIFIER, "x", "x", 1},
		{TokenType::RIGHT_PAREN, ")", ")", 1},
		{TokenType::LEFT_BRACE, "{", "{", 1},
		{TokenType::RETURN, "return", "return", 1},
		{TokenType::IDENTIFIER, "x", "x", 1},
		{TokenType::SEMICOLON, ";", ";", 1},
		{TokenType::RIGHT_BRACE, "}", "}", 1},
		{TokenType::END_OF_FILE, "\n", "\n", 1}
	});

	ASSERT_EQ(program.size(), 1);
	const FunctionDeclStmt* fn = dynamic_cast<const FunctionDeclStmt*>(program[0].get());
	ASSERT_NE(fn, nullptr);
	ASSERT_EQ(fn->getParameters().size(), 1u);
	EXPECT_EQ(fn->getParameters()[0].getLexeme(), "x");

	ASSERT_EQ(fn->getBody()->getStatements().size(), 1u);
	const ReturnStmt* return_stmt = dynamic_cast<const ReturnStmt*>(fn->getBody()->getStatements()[0].get());
	ASSERT_NE(return_stmt, nullptr);
	ASSERT_NO_FATAL_FAILURE(expectVariable(return_stmt->getValue(), "x"));
}

TEST_F(ParserTestFixture, FunctionDeclStmtMultipleParamsPassed) {
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
	const FunctionDeclStmt* fn = dynamic_cast<const FunctionDeclStmt*>(program[0].get());
	ASSERT_NE(fn, nullptr);
	ASSERT_EQ(fn->getParameters().size(), 2u);
	EXPECT_EQ(fn->getParameters()[0].getLexeme(), "a");
	EXPECT_EQ(fn->getParameters()[1].getLexeme(), "b");

	ASSERT_EQ(fn->getBody()->getStatements().size(), 1u);
	const ReturnStmt* return_stmt = dynamic_cast<const ReturnStmt*>(fn->getBody()->getStatements()[0].get());
	ASSERT_NE(return_stmt, nullptr);
	ASSERT_NO_FATAL_FAILURE(expectBinary(return_stmt->getValue(), TokenType::PLUS, "+"));
}

TEST_F(ParserTestFixture, FunctionDeclStmtMissingNameFailed) {
	// Func (a) { return a; }
	expectParseThrows({
		{TokenType::FUNC, "Func", "Func", 1},
		{TokenType::LEFT_PAREN, "(", "(", 1},
		{TokenType::IDENTIFIER, "a", "a", 1},
		{TokenType::RIGHT_PAREN, ")", ")", 1},
		{TokenType::LEFT_BRACE, "{", "{", 1},
		{TokenType::RETURN, "return", "return", 1},
		{TokenType::IDENTIFIER, "a", "a", 1},
		{TokenType::SEMICOLON, ";", ";", 1},
		{TokenType::RIGHT_BRACE, "}", "}", 1},
		{TokenType::END_OF_FILE, "\n", "\n", 1}
	});
}

TEST_F(ParserTestFixture, FunctionDeclStmtMissingLeftParenFailed) {
	// Func add a, b) { return a+b; }
	expectParseThrows({
		{TokenType::FUNC, "Func", "Func", 1},
		{TokenType::IDENTIFIER, "add", "add", 1},
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
}

TEST_F(ParserTestFixture, FunctionDeclStmtMissingRightParenFailed) {
	// Func add(a, b { return a+b; }
	expectParseThrows({
		{TokenType::FUNC, "Func", "Func", 1},
		{TokenType::IDENTIFIER, "add", "add", 1},
		{TokenType::LEFT_PAREN, "(", "(", 1},
		{TokenType::IDENTIFIER, "a", "a", 1},
		{TokenType::COMMA, ",", ",", 1},
		{TokenType::IDENTIFIER, "b", "b", 1},
		{TokenType::LEFT_BRACE, "{", "{", 1},
		{TokenType::RETURN, "return", "return", 1},
		{TokenType::IDENTIFIER, "a", "a", 1},
		{TokenType::PLUS, "+", "+", 1},
		{TokenType::IDENTIFIER, "b", "b", 1},
		{TokenType::SEMICOLON, ";", ";", 1},
		{TokenType::RIGHT_BRACE, "}", "}", 1},
		{TokenType::END_OF_FILE, "\n", "\n", 1}
	});
}

TEST_F(ParserTestFixture, FunctionDeclStmtMissingBodyBraceFailed) {
	// Func add(a, b) return a+b;
	expectParseThrows({
		{TokenType::FUNC, "Func", "Func", 1},
		{TokenType::IDENTIFIER, "add", "add", 1},
		{TokenType::LEFT_PAREN, "(", "(", 1},
		{TokenType::IDENTIFIER, "a", "a", 1},
		{TokenType::COMMA, ",", ",", 1},
		{TokenType::IDENTIFIER, "b", "b", 1},
		{TokenType::RIGHT_PAREN, ")", ")", 1},
		{TokenType::RETURN, "return", "return", 1},
		{TokenType::IDENTIFIER, "a", "a", 1},
		{TokenType::PLUS, "+", "+", 1},
		{TokenType::IDENTIFIER, "b", "b", 1},
		{TokenType::SEMICOLON, ";", ";", 1},
		{TokenType::END_OF_FILE, "\n", "\n", 1}
	});
}

TEST_F(ParserTestFixture, FunctionDeclStmtMissingCommaBetweenParamsFailed) {
	// Func add(a b) { return a+b; }
	expectParseThrows({
		{TokenType::FUNC, "Func", "Func", 1},
		{TokenType::IDENTIFIER, "add", "add", 1},
		{TokenType::LEFT_PAREN, "(", "(", 1},
		{TokenType::IDENTIFIER, "a", "a", 1},
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
}

TEST_F(ParserTestFixture, ReturnStmtOutsideFunctionFailed) {
	// return 1;
	expectParseThrows({
		{TokenType::RETURN, "return", "return", 1},
		{TokenType::NUMBER, "1", 1.0, 1},
		{TokenType::SEMICOLON, ";", ";", 1},
		{TokenType::END_OF_FILE, "\n", "\n", 1}
	});
}

TEST_F(ParserTestFixture, ReturnStmtAfterFunctionDeclEndsOutsideFunctionFailed) {
	// Func f() { return 1; } return 2;
	expectParseThrows({
		{TokenType::FUNC, "Func", "Func", 1},
		{TokenType::IDENTIFIER, "f", "f", 1},
		{TokenType::LEFT_PAREN, "(", "(", 1},
		{TokenType::RIGHT_PAREN, ")", ")", 1},
		{TokenType::LEFT_BRACE, "{", "{", 1},
		{TokenType::RETURN, "return", "return", 1},
		{TokenType::NUMBER, "1", 1.0, 1},
		{TokenType::SEMICOLON, ";", ";", 1},
		{TokenType::RIGHT_BRACE, "}", "}", 1},
		{TokenType::RETURN, "return", "return", 1},
		{TokenType::NUMBER, "2", 2.0, 1},
		{TokenType::SEMICOLON, ";", ";", 1},
		{TokenType::END_OF_FILE, "\n", "\n", 1}
	});
}

TEST_F(ParserTestFixture, ReturnStmtInsideNestedBlockInsideFunctionPassed) {
	// Func f() { { return 1; } }
	const auto& program = buildAndParseProgram({
		{TokenType::FUNC, "Func", "Func", 1},
		{TokenType::IDENTIFIER, "f", "f", 1},
		{TokenType::LEFT_PAREN, "(", "(", 1},
		{TokenType::RIGHT_PAREN, ")", ")", 1},
		{TokenType::LEFT_BRACE, "{", "{", 1},
		{TokenType::LEFT_BRACE, "{", "{", 1},
		{TokenType::RETURN, "return", "return", 1},
		{TokenType::NUMBER, "1", 1.0, 1},
		{TokenType::SEMICOLON, ";", ";", 1},
		{TokenType::RIGHT_BRACE, "}", "}", 1},
		{TokenType::RIGHT_BRACE, "}", "}", 1},
		{TokenType::END_OF_FILE, "\n", "\n", 1}
	});

	ASSERT_EQ(program.size(), 1);
	const FunctionDeclStmt* fn = dynamic_cast<const FunctionDeclStmt*>(program[0].get());
	ASSERT_NE(fn, nullptr);

	ASSERT_EQ(fn->getBody()->getStatements().size(), 1u);
	const BlockStmt* nested_block = dynamic_cast<const BlockStmt*>(fn->getBody()->getStatements()[0].get());
	ASSERT_NE(nested_block, nullptr);
	ASSERT_EQ(nested_block->getStatements().size(), 1u);
	EXPECT_NE(dynamic_cast<const ReturnStmt*>(nested_block->getStatements()[0].get()), nullptr);
}

TEST_F(ParserTestFixture, CallExprNoArgsPassed) {
	// add();
	const auto& program = buildAndParseProgram({
		{TokenType::IDENTIFIER, "add", "add", 1},
		{TokenType::LEFT_PAREN, "(", "(", 1},
		{TokenType::RIGHT_PAREN, ")", ")", 1},
		{TokenType::SEMICOLON, ";", ";", 1},
		{TokenType::END_OF_FILE, "\n", "\n", 1}
	});

	ASSERT_EQ(program.size(), 1);
	const ExpressionStmt* expr_stmt = dynamic_cast<const ExpressionStmt*>(program[0].get());
	ASSERT_NE(expr_stmt, nullptr);

	const CallExpr* call_expr = expectCall(expr_stmt->getExpr());
	ASSERT_NE(call_expr, nullptr);
	ASSERT_NO_FATAL_FAILURE(expectVariable(call_expr->getCallee(), "add"));
	EXPECT_EQ(call_expr->getArguments().size(), 0u);
}

TEST_F(ParserTestFixture, CallExprSingleArgPassed) {
	// add(1);
	const auto& program = buildAndParseProgram({
		{TokenType::IDENTIFIER, "add", "add", 1},
		{TokenType::LEFT_PAREN, "(", "(", 1},
		{TokenType::NUMBER, "1", 1.0, 1},
		{TokenType::RIGHT_PAREN, ")", ")", 1},
		{TokenType::SEMICOLON, ";", ";", 1},
		{TokenType::END_OF_FILE, "\n", "\n", 1}
	});

	ASSERT_EQ(program.size(), 1);
	const ExpressionStmt* expr_stmt = dynamic_cast<const ExpressionStmt*>(program[0].get());
	ASSERT_NE(expr_stmt, nullptr);

	const CallExpr* call_expr = expectCall(expr_stmt->getExpr());
	ASSERT_NE(call_expr, nullptr);
	ASSERT_EQ(call_expr->getArguments().size(), 1u);
	ASSERT_NO_FATAL_FAILURE(expectLiteral(call_expr->getArguments()[0].get(), TokenType::NUMBER, "1"));
}

TEST_F(ParserTestFixture, CallExprMultipleArgsPassed) {
	// add(1, 2, 3);
	const auto& program = buildAndParseProgram({
		{TokenType::IDENTIFIER, "add", "add", 1},
		{TokenType::LEFT_PAREN, "(", "(", 1},
		{TokenType::NUMBER, "1", 1.0, 1},
		{TokenType::COMMA, ",", ",", 1},
		{TokenType::NUMBER, "2", 2.0, 1},
		{TokenType::COMMA, ",", ",", 1},
		{TokenType::NUMBER, "3", 3.0, 1},
		{TokenType::RIGHT_PAREN, ")", ")", 1},
		{TokenType::SEMICOLON, ";", ";", 1},
		{TokenType::END_OF_FILE, "\n", "\n", 1}
	});

	ASSERT_EQ(program.size(), 1);
	const ExpressionStmt* expr_stmt = dynamic_cast<const ExpressionStmt*>(program[0].get());
	ASSERT_NE(expr_stmt, nullptr);

	const CallExpr* call_expr = expectCall(expr_stmt->getExpr());
	ASSERT_NE(call_expr, nullptr);
	ASSERT_EQ(call_expr->getArguments().size(), 3u);
	ASSERT_NO_FATAL_FAILURE(expectLiteral(call_expr->getArguments()[0].get(), TokenType::NUMBER, "1"));
	ASSERT_NO_FATAL_FAILURE(expectLiteral(call_expr->getArguments()[1].get(), TokenType::NUMBER, "2"));
	ASSERT_NO_FATAL_FAILURE(expectLiteral(call_expr->getArguments()[2].get(), TokenType::NUMBER, "3"));
}

TEST_F(ParserTestFixture, CallExprChainedPassed) {
	// f()();
	const auto& program = buildAndParseProgram({
		{TokenType::IDENTIFIER, "f", "f", 1},
		{TokenType::LEFT_PAREN, "(", "(", 1},
		{TokenType::RIGHT_PAREN, ")", ")", 1},
		{TokenType::LEFT_PAREN, "(", "(", 1},
		{TokenType::RIGHT_PAREN, ")", ")", 1},
		{TokenType::SEMICOLON, ";", ";", 1},
		{TokenType::END_OF_FILE, "\n", "\n", 1}
	});

	ASSERT_EQ(program.size(), 1);
	const ExpressionStmt* expr_stmt = dynamic_cast<const ExpressionStmt*>(program[0].get());
	ASSERT_NE(expr_stmt, nullptr);

	const CallExpr* outer = expectCall(expr_stmt->getExpr());
	ASSERT_NE(outer, nullptr);
	EXPECT_EQ(outer->getArguments().size(), 0u);

	const CallExpr* inner = expectCall(outer->getCallee());
	ASSERT_NE(inner, nullptr);
	EXPECT_EQ(inner->getArguments().size(), 0u);
	ASSERT_NO_FATAL_FAILURE(expectVariable(inner->getCallee(), "f"));
}

TEST_F(ParserTestFixture, CallExprAssignedToVariablePassed) {
	// var ret = add(1, 2);
	VarDeclareStmt* stmt = buildAndParseVarDeclareStmt({
		{TokenType::IDENTIFIER, "add", "add", 1},
		{TokenType::LEFT_PAREN, "(", "(", 1},
		{TokenType::NUMBER, "1", 1.0, 1},
		{TokenType::COMMA, ",", ",", 1},
		{TokenType::NUMBER, "2", 2.0, 1},
		{TokenType::RIGHT_PAREN, ")", ")", 1}
	});

	ASSERT_NO_FATAL_FAILURE(expectDeclaredName(stmt, "a"));
	const CallExpr* call_expr = expectCall(stmt->getInitializer());
	ASSERT_NE(call_expr, nullptr);
	ASSERT_NO_FATAL_FAILURE(expectVariable(call_expr->getCallee(), "add"));
	EXPECT_EQ(call_expr->getArguments().size(), 2u);
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

TEST_F(ParserTestFixture, CallExprMissingCommaFailed) {
	// add(1 2);
	expectParseThrows({
		{TokenType::IDENTIFIER, "add", "add", 1},
		{TokenType::LEFT_PAREN, "(", "(", 1},
		{TokenType::NUMBER, "1", 1.0, 1},
		{TokenType::NUMBER, "2", 2.0, 1},
		{TokenType::RIGHT_PAREN, ")", ")", 1},
		{TokenType::SEMICOLON, ";", ";", 1},
		{TokenType::END_OF_FILE, "\n", "\n", 1}
	});
}

TEST_F(ParserTestFixture, CallExprTrailingCommaFailed) {
	// add(1,);
	expectParseThrows({
		{TokenType::IDENTIFIER, "add", "add", 1},
		{TokenType::LEFT_PAREN, "(", "(", 1},
		{TokenType::NUMBER, "1", 1.0, 1},
		{TokenType::COMMA, ",", ",", 1},
		{TokenType::RIGHT_PAREN, ")", ")", 1},
		{TokenType::SEMICOLON, ";", ";", 1},
		{TokenType::END_OF_FILE, "\n", "\n", 1}
	});
}

TEST_F(ParserTestFixture, RecursiveCallInsideFunctionBodyPassed) {
	// Func fact(n) { return fact(n - 1); }
	const auto& program = buildAndParseProgram({
		{TokenType::FUNC, "Func", "Func", 1},
		{TokenType::IDENTIFIER, "fact", "fact", 1},
		{TokenType::LEFT_PAREN, "(", "(", 1},
		{TokenType::IDENTIFIER, "n", "n", 1},
		{TokenType::RIGHT_PAREN, ")", ")", 1},
		{TokenType::LEFT_BRACE, "{", "{", 1},
		{TokenType::RETURN, "return", "return", 1},
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
	const FunctionDeclStmt* fn = dynamic_cast<const FunctionDeclStmt*>(program[0].get());
	ASSERT_NE(fn, nullptr);

	ASSERT_EQ(fn->getBody()->getStatements().size(), 1u);
	const ReturnStmt* return_stmt = dynamic_cast<const ReturnStmt*>(fn->getBody()->getStatements()[0].get());
	ASSERT_NE(return_stmt, nullptr);

	const CallExpr* call_expr = expectCall(return_stmt->getValue());
	ASSERT_NE(call_expr, nullptr);
	ASSERT_NO_FATAL_FAILURE(expectVariable(call_expr->getCallee(), "fact"));
	ASSERT_EQ(call_expr->getArguments().size(), 1u);
	ASSERT_NO_FATAL_FAILURE(expectBinary(call_expr->getArguments()[0].get(), TokenType::MINUS, "-"));
}

TEST_F(ParserTestFixture, ClassDeclStmtEmptyPassed) {
	// Class Robot { }
	const auto& program = buildAndParseProgram({
		{TokenType::CLASS, "Class", "Class", 1},
		{TokenType::IDENTIFIER, "Robot", "Robot", 1},
		{TokenType::LEFT_BRACE, "{", "{", 1},
		{TokenType::RIGHT_BRACE, "}", "}", 1},
		{TokenType::END_OF_FILE, "\n", "\n", 1}
	});

	ASSERT_EQ(program.size(), 1);
	const ClassDeclStmt* class_decl = dynamic_cast<const ClassDeclStmt*>(program[0].get());
	ASSERT_NE(class_decl, nullptr);
	EXPECT_EQ(class_decl->getName().getLexeme(), "Robot");
	EXPECT_FALSE(class_decl->getSuperclassName().has_value());
	EXPECT_EQ(class_decl->getMethods().size(), 0u);
}

TEST_F(ParserTestFixture, ClassDeclStmtWithSuperclassPassed) {
	// Class SpeedRobot : Robot { }
	const auto& program = buildAndParseProgram({
		{TokenType::CLASS, "Class", "Class", 1},
		{TokenType::IDENTIFIER, "SpeedRobot", "SpeedRobot", 1},
		{TokenType::COLON, ":", ":", 1},
		{TokenType::IDENTIFIER, "Robot", "Robot", 1},
		{TokenType::LEFT_BRACE, "{", "{", 1},
		{TokenType::RIGHT_BRACE, "}", "}", 1},
		{TokenType::END_OF_FILE, "\n", "\n", 1}
	});

	ASSERT_EQ(program.size(), 1);
	const ClassDeclStmt* class_decl = dynamic_cast<const ClassDeclStmt*>(program[0].get());
	ASSERT_NE(class_decl, nullptr);
	ASSERT_TRUE(class_decl->getSuperclassName().has_value());
	EXPECT_EQ(class_decl->getSuperclassName()->getLexeme(), "Robot");
}

TEST_F(ParserTestFixture, ClassDeclStmtWithMethodPassed) {
	// Class Robot { speak() { print this.name; } }
	const auto& program = buildAndParseProgram({
		{TokenType::CLASS, "Class", "Class", 1},
		{TokenType::IDENTIFIER, "Robot", "Robot", 1},
		{TokenType::LEFT_BRACE, "{", "{", 1},
		{TokenType::IDENTIFIER, "speak", "speak", 1},
		{TokenType::LEFT_PAREN, "(", "(", 1},
		{TokenType::RIGHT_PAREN, ")", ")", 1},
		{TokenType::LEFT_BRACE, "{", "{", 1},
		{TokenType::PRINT, "print", "print", 1},
		{TokenType::THIS, "this", "this", 1},
		{TokenType::DOT, ".", ".", 1},
		{TokenType::IDENTIFIER, "name", "name", 1},
		{TokenType::SEMICOLON, ";", ";", 1},
		{TokenType::RIGHT_BRACE, "}", "}", 1},
		{TokenType::RIGHT_BRACE, "}", "}", 1},
		{TokenType::END_OF_FILE, "\n", "\n", 1}
	});

	ASSERT_EQ(program.size(), 1);
	const ClassDeclStmt* class_decl = dynamic_cast<const ClassDeclStmt*>(program[0].get());
	ASSERT_NE(class_decl, nullptr);
	ASSERT_EQ(class_decl->getMethods().size(), 1u);
	EXPECT_EQ(class_decl->getMethods()[0]->getName().getLexeme(), "speak");
	EXPECT_EQ(class_decl->getMethods()[0]->getParameters().size(), 0u);

	ASSERT_EQ(class_decl->getMethods()[0]->getBody()->getStatements().size(), 1u);
	const PrintStmt* print_stmt = dynamic_cast<const PrintStmt*>(class_decl->getMethods()[0]->getBody()->getStatements()[0].get());
	ASSERT_NE(print_stmt, nullptr);
	const GetExpr* get_expr = expectGet(print_stmt->getExpr());
	ASSERT_NE(get_expr, nullptr);
	ASSERT_NO_FATAL_FAILURE(expectThis(get_expr->getObject()));
	EXPECT_EQ(get_expr->getName().getLexeme(), "name");
}

TEST_F(ParserTestFixture, ClassDeclStmtWithInitMethodPassed) {
	// Class Robot { init(name) { this.name = name; } }
	const auto& program = buildAndParseProgram({
		{TokenType::CLASS, "Class", "Class", 1},
		{TokenType::IDENTIFIER, "Robot", "Robot", 1},
		{TokenType::LEFT_BRACE, "{", "{", 1},
		{TokenType::IDENTIFIER, "init", "init", 1},
		{TokenType::LEFT_PAREN, "(", "(", 1},
		{TokenType::IDENTIFIER, "name", "name", 1},
		{TokenType::RIGHT_PAREN, ")", ")", 1},
		{TokenType::LEFT_BRACE, "{", "{", 1},
		{TokenType::THIS, "this", "this", 1},
		{TokenType::DOT, ".", ".", 1},
		{TokenType::IDENTIFIER, "name", "name", 1},
		{TokenType::EQUAL, "=", "=", 1},
		{TokenType::IDENTIFIER, "name", "name", 1},
		{TokenType::SEMICOLON, ";", ";", 1},
		{TokenType::RIGHT_BRACE, "}", "}", 1},
		{TokenType::RIGHT_BRACE, "}", "}", 1},
		{TokenType::END_OF_FILE, "\n", "\n", 1}
	});

	ASSERT_EQ(program.size(), 1);
	const ClassDeclStmt* class_decl = dynamic_cast<const ClassDeclStmt*>(program[0].get());
	ASSERT_NE(class_decl, nullptr);
	ASSERT_EQ(class_decl->getMethods().size(), 1u);
	EXPECT_EQ(class_decl->getMethods()[0]->getName().getLexeme(), "init");
	ASSERT_EQ(class_decl->getMethods()[0]->getParameters().size(), 1u);
	EXPECT_EQ(class_decl->getMethods()[0]->getParameters()[0].getLexeme(), "name");

	ASSERT_EQ(class_decl->getMethods()[0]->getBody()->getStatements().size(), 1u);
	const ExpressionStmt* expr_stmt = dynamic_cast<const ExpressionStmt*>(class_decl->getMethods()[0]->getBody()->getStatements()[0].get());
	ASSERT_NE(expr_stmt, nullptr);
	const SetExpr* set_expr = expectSet(expr_stmt->getExpr());
	ASSERT_NE(set_expr, nullptr);
	ASSERT_NO_FATAL_FAILURE(expectThis(set_expr->getObject()));
	EXPECT_EQ(set_expr->getName().getLexeme(), "name");
	ASSERT_NO_FATAL_FAILURE(expectVariable(set_expr->getValue(), "name"));
}

TEST_F(ParserTestFixture, ClassDeclStmtMultipleMethodsPassed) {
	// Class Robot { init(name) { this.name = name; } speak() { print this.name; } }
	const auto& program = buildAndParseProgram({
		{TokenType::CLASS, "Class", "Class", 1},
		{TokenType::IDENTIFIER, "Robot", "Robot", 1},
		{TokenType::LEFT_BRACE, "{", "{", 1},
		{TokenType::IDENTIFIER, "init", "init", 1},
		{TokenType::LEFT_PAREN, "(", "(", 1},
		{TokenType::IDENTIFIER, "name", "name", 1},
		{TokenType::RIGHT_PAREN, ")", ")", 1},
		{TokenType::LEFT_BRACE, "{", "{", 1},
		{TokenType::THIS, "this", "this", 1},
		{TokenType::DOT, ".", ".", 1},
		{TokenType::IDENTIFIER, "name", "name", 1},
		{TokenType::EQUAL, "=", "=", 1},
		{TokenType::IDENTIFIER, "name", "name", 1},
		{TokenType::SEMICOLON, ";", ";", 1},
		{TokenType::RIGHT_BRACE, "}", "}", 1},
		{TokenType::IDENTIFIER, "speak", "speak", 1},
		{TokenType::LEFT_PAREN, "(", "(", 1},
		{TokenType::RIGHT_PAREN, ")", ")", 1},
		{TokenType::LEFT_BRACE, "{", "{", 1},
		{TokenType::PRINT, "print", "print", 1},
		{TokenType::THIS, "this", "this", 1},
		{TokenType::DOT, ".", ".", 1},
		{TokenType::IDENTIFIER, "name", "name", 1},
		{TokenType::SEMICOLON, ";", ";", 1},
		{TokenType::RIGHT_BRACE, "}", "}", 1},
		{TokenType::RIGHT_BRACE, "}", "}", 1},
		{TokenType::END_OF_FILE, "\n", "\n", 1}
	});

	ASSERT_EQ(program.size(), 1);
	const ClassDeclStmt* class_decl = dynamic_cast<const ClassDeclStmt*>(program[0].get());
	ASSERT_NE(class_decl, nullptr);
	ASSERT_EQ(class_decl->getMethods().size(), 2u);
	EXPECT_EQ(class_decl->getMethods()[0]->getName().getLexeme(), "init");
	EXPECT_EQ(class_decl->getMethods()[1]->getName().getLexeme(), "speak");
}

TEST_F(ParserTestFixture, ClassDeclStmtMissingNameFailed) {
	// Class { }
	expectParseThrows({
		{TokenType::CLASS, "Class", "Class", 1},
		{TokenType::LEFT_BRACE, "{", "{", 1},
		{TokenType::RIGHT_BRACE, "}", "}", 1},
		{TokenType::END_OF_FILE, "\n", "\n", 1}
	});
}

TEST_F(ParserTestFixture, ClassDeclStmtMissingBodyBraceFailed) {
	// Class Robot
	expectParseThrows({
		{TokenType::CLASS, "Class", "Class", 1},
		{TokenType::IDENTIFIER, "Robot", "Robot", 1},
		{TokenType::END_OF_FILE, "\n", "\n", 1}
	});
}

TEST_F(ParserTestFixture, ClassDeclStmtMissingSuperclassNameFailed) {
	// Class SpeedRobot : { }
	expectParseThrows({
		{TokenType::CLASS, "Class", "Class", 1},
		{TokenType::IDENTIFIER, "SpeedRobot", "SpeedRobot", 1},
		{TokenType::COLON, ":", ":", 1},
		{TokenType::LEFT_BRACE, "{", "{", 1},
		{TokenType::RIGHT_BRACE, "}", "}", 1},
		{TokenType::END_OF_FILE, "\n", "\n", 1}
	});
}

TEST_F(ParserTestFixture, GetExprPassed) {
	// print r.name;
	const auto& program = buildAndParseProgram({
		{TokenType::PRINT, "print", "print", 1},
		{TokenType::IDENTIFIER, "r", "r", 1},
		{TokenType::DOT, ".", ".", 1},
		{TokenType::IDENTIFIER, "name", "name", 1},
		{TokenType::SEMICOLON, ";", ";", 1},
		{TokenType::END_OF_FILE, "\n", "\n", 1}
	});

	ASSERT_EQ(program.size(), 1);
	const PrintStmt* print_stmt = dynamic_cast<const PrintStmt*>(program[0].get());
	ASSERT_NE(print_stmt, nullptr);

	const GetExpr* get_expr = expectGet(print_stmt->getExpr());
	ASSERT_NE(get_expr, nullptr);
	ASSERT_NO_FATAL_FAILURE(expectVariable(get_expr->getObject(), "r"));
	EXPECT_EQ(get_expr->getName().getLexeme(), "name");
}

TEST_F(ParserTestFixture, GetExprChainedPassed) {
	// print r.a.b;
	const auto& program = buildAndParseProgram({
		{TokenType::PRINT, "print", "print", 1},
		{TokenType::IDENTIFIER, "r", "r", 1},
		{TokenType::DOT, ".", ".", 1},
		{TokenType::IDENTIFIER, "a", "a", 1},
		{TokenType::DOT, ".", ".", 1},
		{TokenType::IDENTIFIER, "b", "b", 1},
		{TokenType::SEMICOLON, ";", ";", 1},
		{TokenType::END_OF_FILE, "\n", "\n", 1}
	});

	ASSERT_EQ(program.size(), 1);
	const PrintStmt* print_stmt = dynamic_cast<const PrintStmt*>(program[0].get());
	ASSERT_NE(print_stmt, nullptr);

	const GetExpr* outer = expectGet(print_stmt->getExpr());
	ASSERT_NE(outer, nullptr);
	EXPECT_EQ(outer->getName().getLexeme(), "b");

	const GetExpr* inner = expectGet(outer->getObject());
	ASSERT_NE(inner, nullptr);
	EXPECT_EQ(inner->getName().getLexeme(), "a");
	ASSERT_NO_FATAL_FAILURE(expectVariable(inner->getObject(), "r"));
}

TEST_F(ParserTestFixture, SetExprPassed) {
	// r.name = "robot";
	const auto& program = buildAndParseProgram({
		{TokenType::IDENTIFIER, "r", "r", 1},
		{TokenType::DOT, ".", ".", 1},
		{TokenType::IDENTIFIER, "name", "name", 1},
		{TokenType::EQUAL, "=", "=", 1},
		{TokenType::STRING, "\"robot\"", "robot", 1},
		{TokenType::SEMICOLON, ";", ";", 1},
		{TokenType::END_OF_FILE, "\n", "\n", 1}
	});

	ASSERT_EQ(program.size(), 1);
	const ExpressionStmt* expr_stmt = dynamic_cast<const ExpressionStmt*>(program[0].get());
	ASSERT_NE(expr_stmt, nullptr);

	const SetExpr* set_expr = expectSet(expr_stmt->getExpr());
	ASSERT_NE(set_expr, nullptr);
	ASSERT_NO_FATAL_FAILURE(expectVariable(set_expr->getObject(), "r"));
	EXPECT_EQ(set_expr->getName().getLexeme(), "name");
	ASSERT_NO_FATAL_FAILURE(expectLiteral(set_expr->getValue(), TokenType::STRING, "\"robot\""));
}

TEST_F(ParserTestFixture, SetExprUpdateWithSelfReadPassed) {
	// r.speed = r.speed + 1;
	const auto& program = buildAndParseProgram({
		{TokenType::IDENTIFIER, "r", "r", 1},
		{TokenType::DOT, ".", ".", 1},
		{TokenType::IDENTIFIER, "speed", "speed", 1},
		{TokenType::EQUAL, "=", "=", 1},
		{TokenType::IDENTIFIER, "r", "r", 1},
		{TokenType::DOT, ".", ".", 1},
		{TokenType::IDENTIFIER, "speed", "speed", 1},
		{TokenType::PLUS, "+", "+", 1},
		{TokenType::NUMBER, "1", 1.0, 1},
		{TokenType::SEMICOLON, ";", ";", 1},
		{TokenType::END_OF_FILE, "\n", "\n", 1}
	});

	ASSERT_EQ(program.size(), 1);
	const ExpressionStmt* expr_stmt = dynamic_cast<const ExpressionStmt*>(program[0].get());
	ASSERT_NE(expr_stmt, nullptr);

	const SetExpr* set_expr = expectSet(expr_stmt->getExpr());
	ASSERT_NE(set_expr, nullptr);
	EXPECT_EQ(set_expr->getName().getLexeme(), "speed");
	ASSERT_NO_FATAL_FAILURE(expectBinary(set_expr->getValue(), TokenType::PLUS, "+"));
}

TEST_F(ParserTestFixture, ThisExprAsReturnValuePassed) {
	// Class Robot { getSelf() { return this; } }
	const auto& program = buildAndParseProgram({
		{TokenType::CLASS, "Class", "Class", 1},
		{TokenType::IDENTIFIER, "Robot", "Robot", 1},
		{TokenType::LEFT_BRACE, "{", "{", 1},
		{TokenType::IDENTIFIER, "getSelf", "getSelf", 1},
		{TokenType::LEFT_PAREN, "(", "(", 1},
		{TokenType::RIGHT_PAREN, ")", ")", 1},
		{TokenType::LEFT_BRACE, "{", "{", 1},
		{TokenType::RETURN, "return", "return", 1},
		{TokenType::THIS, "this", "this", 1},
		{TokenType::SEMICOLON, ";", ";", 1},
		{TokenType::RIGHT_BRACE, "}", "}", 1},
		{TokenType::RIGHT_BRACE, "}", "}", 1},
		{TokenType::END_OF_FILE, "\n", "\n", 1}
	});

	ASSERT_EQ(program.size(), 1);
	const ClassDeclStmt* class_decl = dynamic_cast<const ClassDeclStmt*>(program[0].get());
	ASSERT_NE(class_decl, nullptr);
	ASSERT_EQ(class_decl->getMethods().size(), 1u);

	ASSERT_EQ(class_decl->getMethods()[0]->getBody()->getStatements().size(), 1u);
	const ReturnStmt* return_stmt = dynamic_cast<const ReturnStmt*>(class_decl->getMethods()[0]->getBody()->getStatements()[0].get());
	ASSERT_NE(return_stmt, nullptr);
	ASSERT_NO_FATAL_FAILURE(expectThis(return_stmt->getValue()));
}

TEST_F(ParserTestFixture, MethodCallOnThisPassed) {
	// Class Robot { speak() { this.other(); } }
	const auto& program = buildAndParseProgram({
		{TokenType::CLASS, "Class", "Class", 1},
		{TokenType::IDENTIFIER, "Robot", "Robot", 1},
		{TokenType::LEFT_BRACE, "{", "{", 1},
		{TokenType::IDENTIFIER, "speak", "speak", 1},
		{TokenType::LEFT_PAREN, "(", "(", 1},
		{TokenType::RIGHT_PAREN, ")", ")", 1},
		{TokenType::LEFT_BRACE, "{", "{", 1},
		{TokenType::THIS, "this", "this", 1},
		{TokenType::DOT, ".", ".", 1},
		{TokenType::IDENTIFIER, "other", "other", 1},
		{TokenType::LEFT_PAREN, "(", "(", 1},
		{TokenType::RIGHT_PAREN, ")", ")", 1},
		{TokenType::SEMICOLON, ";", ";", 1},
		{TokenType::RIGHT_BRACE, "}", "}", 1},
		{TokenType::RIGHT_BRACE, "}", "}", 1},
		{TokenType::END_OF_FILE, "\n", "\n", 1}
	});

	ASSERT_EQ(program.size(), 1);
	const ClassDeclStmt* class_decl = dynamic_cast<const ClassDeclStmt*>(program[0].get());
	ASSERT_NE(class_decl, nullptr);

	ASSERT_EQ(class_decl->getMethods()[0]->getBody()->getStatements().size(), 1u);
	const ExpressionStmt* expr_stmt = dynamic_cast<const ExpressionStmt*>(class_decl->getMethods()[0]->getBody()->getStatements()[0].get());
	ASSERT_NE(expr_stmt, nullptr);

	const CallExpr* call_expr = expectCall(expr_stmt->getExpr());
	ASSERT_NE(call_expr, nullptr);
	const GetExpr* get_expr = expectGet(call_expr->getCallee());
	ASSERT_NE(get_expr, nullptr);
	ASSERT_NO_FATAL_FAILURE(expectThis(get_expr->getObject()));
	EXPECT_EQ(get_expr->getName().getLexeme(), "other");
}

TEST_F(ParserTestFixture, SuperMethodCallPassed) {
	// Class SpeedRobot : Robot { speak() { Super.speak(); } }
	const auto& program = buildAndParseProgram({
		{TokenType::CLASS, "Class", "Class", 1},
		{TokenType::IDENTIFIER, "SpeedRobot", "SpeedRobot", 1},
		{TokenType::COLON, ":", ":", 1},
		{TokenType::IDENTIFIER, "Robot", "Robot", 1},
		{TokenType::LEFT_BRACE, "{", "{", 1},
		{TokenType::IDENTIFIER, "speak", "speak", 1},
		{TokenType::LEFT_PAREN, "(", "(", 1},
		{TokenType::RIGHT_PAREN, ")", ")", 1},
		{TokenType::LEFT_BRACE, "{", "{", 1},
		{TokenType::SUPER, "Super", "Super", 1},
		{TokenType::DOT, ".", ".", 1},
		{TokenType::IDENTIFIER, "speak", "speak", 1},
		{TokenType::LEFT_PAREN, "(", "(", 1},
		{TokenType::RIGHT_PAREN, ")", ")", 1},
		{TokenType::SEMICOLON, ";", ";", 1},
		{TokenType::RIGHT_BRACE, "}", "}", 1},
		{TokenType::RIGHT_BRACE, "}", "}", 1},
		{TokenType::END_OF_FILE, "\n", "\n", 1}
	});

	ASSERT_EQ(program.size(), 1);
	const ClassDeclStmt* class_decl = dynamic_cast<const ClassDeclStmt*>(program[0].get());
	ASSERT_NE(class_decl, nullptr);

	ASSERT_EQ(class_decl->getMethods()[0]->getBody()->getStatements().size(), 1u);
	const ExpressionStmt* expr_stmt = dynamic_cast<const ExpressionStmt*>(class_decl->getMethods()[0]->getBody()->getStatements()[0].get());
	ASSERT_NE(expr_stmt, nullptr);

	const CallExpr* call_expr = expectCall(expr_stmt->getExpr());
	ASSERT_NE(call_expr, nullptr);
	const SuperExpr* super_expr = expectSuper(call_expr->getCallee());
	ASSERT_NE(super_expr, nullptr);
	EXPECT_EQ(super_expr->getMethod().getLexeme(), "speak");
}

TEST_F(ParserTestFixture, SuperFieldAccessPassed) {
	// Class SpeedRobot : Robot { getName() { return Super.name; } }
	const auto& program = buildAndParseProgram({
		{TokenType::CLASS, "Class", "Class", 1},
		{TokenType::IDENTIFIER, "SpeedRobot", "SpeedRobot", 1},
		{TokenType::COLON, ":", ":", 1},
		{TokenType::IDENTIFIER, "Robot", "Robot", 1},
		{TokenType::LEFT_BRACE, "{", "{", 1},
		{TokenType::IDENTIFIER, "getName", "getName", 1},
		{TokenType::LEFT_PAREN, "(", "(", 1},
		{TokenType::RIGHT_PAREN, ")", ")", 1},
		{TokenType::LEFT_BRACE, "{", "{", 1},
		{TokenType::RETURN, "return", "return", 1},
		{TokenType::SUPER, "Super", "Super", 1},
		{TokenType::DOT, ".", ".", 1},
		{TokenType::IDENTIFIER, "name", "name", 1},
		{TokenType::SEMICOLON, ";", ";", 1},
		{TokenType::RIGHT_BRACE, "}", "}", 1},
		{TokenType::RIGHT_BRACE, "}", "}", 1},
		{TokenType::END_OF_FILE, "\n", "\n", 1}
	});

	ASSERT_EQ(program.size(), 1);
	const ClassDeclStmt* class_decl = dynamic_cast<const ClassDeclStmt*>(program[0].get());
	ASSERT_NE(class_decl, nullptr);

	ASSERT_EQ(class_decl->getMethods()[0]->getBody()->getStatements().size(), 1u);
	const ReturnStmt* return_stmt = dynamic_cast<const ReturnStmt*>(class_decl->getMethods()[0]->getBody()->getStatements()[0].get());
	ASSERT_NE(return_stmt, nullptr);

	const SuperExpr* super_expr = expectSuper(return_stmt->getValue());
	ASSERT_NE(super_expr, nullptr);
	EXPECT_EQ(super_expr->getMethod().getLexeme(), "name");
}

TEST_F(ParserTestFixture, SuperMissingDotFailed) {
	// Super speak();
	expectParseThrows({
		{TokenType::SUPER, "Super", "Super", 1},
		{TokenType::IDENTIFIER, "speak", "speak", 1},
		{TokenType::LEFT_PAREN, "(", "(", 1},
		{TokenType::RIGHT_PAREN, ")", ")", 1},
		{TokenType::SEMICOLON, ";", ";", 1},
		{TokenType::END_OF_FILE, "\n", "\n", 1}
	});
}

TEST_F(ParserTestFixture, InstanceOfExprPassed) {
	// print robot instanceof Robot;
	const auto& program = buildAndParseProgram({
		{TokenType::PRINT, "print", "print", 1},
		{TokenType::IDENTIFIER, "robot", "robot", 1},
		{TokenType::INSTANCEOF, "instanceof", "instanceof", 1},
		{TokenType::IDENTIFIER, "Robot", "Robot", 1},
		{TokenType::SEMICOLON, ";", ";", 1},
		{TokenType::END_OF_FILE, "\n", "\n", 1}
	});

	ASSERT_EQ(program.size(), 1);
	const PrintStmt* print_stmt = dynamic_cast<const PrintStmt*>(program[0].get());
	ASSERT_NE(print_stmt, nullptr);

	const InstanceOfExpr* instance_of_expr = expectInstanceOf(print_stmt->getExpr());
	ASSERT_NE(instance_of_expr, nullptr);
	ASSERT_NO_FATAL_FAILURE(expectVariable(instance_of_expr->getObject(), "robot"));
	EXPECT_EQ(instance_of_expr->getClassName().getLexeme(), "Robot");
}

TEST_F(ParserTestFixture, InstanceOfExprMissingClassNameFailed) {
	// print robot instanceof;
	expectParseThrows({
		{TokenType::PRINT, "print", "print", 1},
		{TokenType::IDENTIFIER, "robot", "robot", 1},
		{TokenType::INSTANCEOF, "instanceof", "instanceof", 1},
		{TokenType::SEMICOLON, ";", ";", 1},
		{TokenType::END_OF_FILE, "\n", "\n", 1}
	});
}

TEST_F(ParserTestFixture, InstantiationViaCallExprPassed) {
	// var robot = Robot();
	VarDeclareStmt* stmt = buildAndParseVarDeclareStmt({
		{TokenType::IDENTIFIER, "Robot", "Robot", 1},
		{TokenType::LEFT_PAREN, "(", "(", 1},
		{TokenType::RIGHT_PAREN, ")", ")", 1}
	});

	ASSERT_NO_FATAL_FAILURE(expectDeclaredName(stmt, "a"));
	const CallExpr* call_expr = expectCall(stmt->getInitializer());
	ASSERT_NE(call_expr, nullptr);
	ASSERT_NO_FATAL_FAILURE(expectVariable(call_expr->getCallee(), "Robot"));
	EXPECT_EQ(call_expr->getArguments().size(), 0u);
}

TEST_F(ParserTestFixture, SetExprMissingValueFailed) {
	// r.name = ;
	expectParseThrows({
		{TokenType::IDENTIFIER, "r", "r", 1},
		{TokenType::DOT, ".", ".", 1},
		{TokenType::IDENTIFIER, "name", "name", 1},
		{TokenType::EQUAL, "=", "=", 1},
		{TokenType::SEMICOLON, ";", ";", 1},
		{TokenType::END_OF_FILE, "\n", "\n", 1}
	});
}

TEST_F(ParserTestFixture, GetExprMissingNameAfterDotFailed) {
	// print r.;
	expectParseThrows({
		{TokenType::PRINT, "print", "print", 1},
		{TokenType::IDENTIFIER, "r", "r", 1},
		{TokenType::DOT, ".", ".", 1},
		{TokenType::SEMICOLON, ";", ";", 1},
		{TokenType::END_OF_FILE, "\n", "\n", 1}
	});
}

TEST_F(ParserTestFixture, ImportStmtPassed) {
	// import "sum.txt" alias sum;
	const auto& program = buildAndParseProgram({
		{TokenType::IMPORT, "import", "import", 1},
		{TokenType::STRING, "\"sum.txt\"", "sum.txt", 1},
		{TokenType::ALIAS, "alias", "alias", 1},
		{TokenType::IDENTIFIER, "sum", "sum", 1},
		{TokenType::SEMICOLON, ";", ";", 1},
		{TokenType::END_OF_FILE, "\n", "\n", 1}
	});

	ASSERT_EQ(program.size(), 1);
	const ImportStmt* import_stmt = dynamic_cast<const ImportStmt*>(program[0].get());
	ASSERT_NE(import_stmt, nullptr);
	EXPECT_EQ(import_stmt->getPath().getType(), TokenType::STRING);
	EXPECT_EQ(import_stmt->getPath().getLexeme(), "\"sum.txt\"");
	EXPECT_EQ(import_stmt->getAliasName().getLexeme(), "sum");
}

TEST_F(ParserTestFixture, ImportStmtMissingPathFailed) {
	// import alias sum;
	expectParseThrows({
		{TokenType::IMPORT, "import", "import", 1},
		{TokenType::ALIAS, "alias", "alias", 1},
		{TokenType::IDENTIFIER, "sum", "sum", 1},
		{TokenType::SEMICOLON, ";", ";", 1},
		{TokenType::END_OF_FILE, "\n", "\n", 1}
	});
}

TEST_F(ParserTestFixture, ImportStmtMissingAliasKeywordFailed) {
	// import "sum.txt" sum;
	expectParseThrows({
		{TokenType::IMPORT, "import", "import", 1},
		{TokenType::STRING, "\"sum.txt\"", "sum.txt", 1},
		{TokenType::IDENTIFIER, "sum", "sum", 1},
		{TokenType::SEMICOLON, ";", ";", 1},
		{TokenType::END_OF_FILE, "\n", "\n", 1}
	});
}

TEST_F(ParserTestFixture, ImportStmtMissingAliasNameFailed) {
	// import "sum.txt" alias;
	expectParseThrows({
		{TokenType::IMPORT, "import", "import", 1},
		{TokenType::STRING, "\"sum.txt\"", "sum.txt", 1},
		{TokenType::ALIAS, "alias", "alias", 1},
		{TokenType::SEMICOLON, ";", ";", 1},
		{TokenType::END_OF_FILE, "\n", "\n", 1}
	});
}

TEST_F(ParserTestFixture, ImportStmtMissingSemicolonFailed) {
	// import "sum.txt" alias sum
	expectParseThrows({
		{TokenType::IMPORT, "import", "import", 1},
		{TokenType::STRING, "\"sum.txt\"", "sum.txt", 1},
		{TokenType::ALIAS, "alias", "alias", 1},
		{TokenType::IDENTIFIER, "sum", "sum", 1},
		{TokenType::END_OF_FILE, "\n", "\n", 1}
	});
}

TEST_F(ParserTestFixture, ImportedModuleMemberCallPassed) {
	// sum.add(1, 2);
	const auto& program = buildAndParseProgram({
		{TokenType::IDENTIFIER, "sum", "sum", 1},
		{TokenType::DOT, ".", ".", 1},
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

	const CallExpr* call_expr = expectCall(expr_stmt->getExpr());
	ASSERT_NE(call_expr, nullptr);
	const GetExpr* get_expr = expectGet(call_expr->getCallee());
	ASSERT_NE(get_expr, nullptr);
	ASSERT_NO_FATAL_FAILURE(expectVariable(get_expr->getObject(), "sum"));
	EXPECT_EQ(get_expr->getName().getLexeme(), "add");
	ASSERT_EQ(call_expr->getArguments().size(), 2u);
}

TEST_F(ParserTestFixture, ThisExprOutsideMethodFailed) {
	// this;
	expectParseThrows({
		{TokenType::THIS, "this", "this", 1},
		{TokenType::SEMICOLON, ";", ";", 1},
		{TokenType::END_OF_FILE, "\n", "\n", 1}
	});
}

TEST_F(ParserTestFixture, SuperOutsideMethodFailed) {
	// Super.speak();
	expectParseThrows({
		{TokenType::SUPER, "Super", "Super", 1},
		{TokenType::DOT, ".", ".", 1},
		{TokenType::IDENTIFIER, "speak", "speak", 1},
		{TokenType::LEFT_PAREN, "(", "(", 1},
		{TokenType::RIGHT_PAREN, ")", ")", 1},
		{TokenType::SEMICOLON, ";", ";", 1},
		{TokenType::END_OF_FILE, "\n", "\n", 1}
	});
}

TEST_F(ParserTestFixture, ThisExprInsidePlainFuncFailed) {
	// Func f() { return this; }
	expectParseThrows({
		{TokenType::FUNC, "Func", "Func", 1},
		{TokenType::IDENTIFIER, "f", "f", 1},
		{TokenType::LEFT_PAREN, "(", "(", 1},
		{TokenType::RIGHT_PAREN, ")", ")", 1},
		{TokenType::LEFT_BRACE, "{", "{", 1},
		{TokenType::RETURN, "return", "return", 1},
		{TokenType::THIS, "this", "this", 1},
		{TokenType::SEMICOLON, ";", ";", 1},
		{TokenType::RIGHT_BRACE, "}", "}", 1},
		{TokenType::END_OF_FILE, "\n", "\n", 1}
	});
}

TEST_F(ParserTestFixture, ImportInsideForLoopFailed) {
	// for (var i = 0; i < 1; i = i + 1) { import "x" alias y; }
	expectParseThrows({
		{TokenType::FOR, "for", "for", 1},
		{TokenType::LEFT_PAREN, "(", "(", 1},
		{TokenType::VAR, "var", "var", 1},
		{TokenType::IDENTIFIER, "i", "i", 1},
		{TokenType::EQUAL, "=", "=", 1},
		{TokenType::NUMBER, "0", 0.0, 1},
		{TokenType::SEMICOLON, ";", ";", 1},
		{TokenType::IDENTIFIER, "i", "i", 1},
		{TokenType::LESS, "<", "<", 1},
		{TokenType::NUMBER, "1", 1.0, 1},
		{TokenType::SEMICOLON, ";", ";", 1},
		{TokenType::IDENTIFIER, "i", "i", 1},
		{TokenType::EQUAL, "=", "=", 1},
		{TokenType::IDENTIFIER, "i", "i", 1},
		{TokenType::PLUS, "+", "+", 1},
		{TokenType::NUMBER, "1", 1.0, 1},
		{TokenType::RIGHT_PAREN, ")", ")", 1},
		{TokenType::LEFT_BRACE, "{", "{", 1},
		{TokenType::IMPORT, "import", "import", 1},
		{TokenType::STRING, "\"x\"", "x", 1},
		{TokenType::ALIAS, "alias", "alias", 1},
		{TokenType::IDENTIFIER, "y", "y", 1},
		{TokenType::SEMICOLON, ";", ";", 1},
		{TokenType::RIGHT_BRACE, "}", "}", 1},
		{TokenType::END_OF_FILE, "\n", "\n", 1}
	});
}

TEST_F(ParserTestFixture, ImportInsideNestedBlockInsideForLoopFailed) {
	// for (var i = 0; i < 1; i = i + 1) { { import "x" alias y; } }
	expectParseThrows({
		{TokenType::FOR, "for", "for", 1},
		{TokenType::LEFT_PAREN, "(", "(", 1},
		{TokenType::VAR, "var", "var", 1},
		{TokenType::IDENTIFIER, "i", "i", 1},
		{TokenType::EQUAL, "=", "=", 1},
		{TokenType::NUMBER, "0", 0.0, 1},
		{TokenType::SEMICOLON, ";", ";", 1},
		{TokenType::IDENTIFIER, "i", "i", 1},
		{TokenType::LESS, "<", "<", 1},
		{TokenType::NUMBER, "1", 1.0, 1},
		{TokenType::SEMICOLON, ";", ";", 1},
		{TokenType::IDENTIFIER, "i", "i", 1},
		{TokenType::EQUAL, "=", "=", 1},
		{TokenType::IDENTIFIER, "i", "i", 1},
		{TokenType::PLUS, "+", "+", 1},
		{TokenType::NUMBER, "1", 1.0, 1},
		{TokenType::RIGHT_PAREN, ")", ")", 1},
		{TokenType::LEFT_BRACE, "{", "{", 1},
		{TokenType::LEFT_BRACE, "{", "{", 1},
		{TokenType::IMPORT, "import", "import", 1},
		{TokenType::STRING, "\"x\"", "x", 1},
		{TokenType::ALIAS, "alias", "alias", 1},
		{TokenType::IDENTIFIER, "y", "y", 1},
		{TokenType::SEMICOLON, ";", ";", 1},
		{TokenType::RIGHT_BRACE, "}", "}", 1},
		{TokenType::RIGHT_BRACE, "}", "}", 1},
		{TokenType::END_OF_FILE, "\n", "\n", 1}
	});
}

TEST_F(ParserTestFixture, ImportInsideFuncInsideForLoopFailed) {
	// for (var i = 0; i < 1; i = i + 1) { Func f() { import "x" alias y; } }
	expectParseThrows({
		{TokenType::FOR, "for", "for", 1},
		{TokenType::LEFT_PAREN, "(", "(", 1},
		{TokenType::VAR, "var", "var", 1},
		{TokenType::IDENTIFIER, "i", "i", 1},
		{TokenType::EQUAL, "=", "=", 1},
		{TokenType::NUMBER, "0", 0.0, 1},
		{TokenType::SEMICOLON, ";", ";", 1},
		{TokenType::IDENTIFIER, "i", "i", 1},
		{TokenType::LESS, "<", "<", 1},
		{TokenType::NUMBER, "1", 1.0, 1},
		{TokenType::SEMICOLON, ";", ";", 1},
		{TokenType::IDENTIFIER, "i", "i", 1},
		{TokenType::EQUAL, "=", "=", 1},
		{TokenType::IDENTIFIER, "i", "i", 1},
		{TokenType::PLUS, "+", "+", 1},
		{TokenType::NUMBER, "1", 1.0, 1},
		{TokenType::RIGHT_PAREN, ")", ")", 1},
		{TokenType::LEFT_BRACE, "{", "{", 1},
		{TokenType::FUNC, "Func", "Func", 1},
		{TokenType::IDENTIFIER, "f", "f", 1},
		{TokenType::LEFT_PAREN, "(", "(", 1},
		{TokenType::RIGHT_PAREN, ")", ")", 1},
		{TokenType::LEFT_BRACE, "{", "{", 1},
		{TokenType::IMPORT, "import", "import", 1},
		{TokenType::STRING, "\"x\"", "x", 1},
		{TokenType::ALIAS, "alias", "alias", 1},
		{TokenType::IDENTIFIER, "y", "y", 1},
		{TokenType::SEMICOLON, ";", ";", 1},
		{TokenType::RIGHT_BRACE, "}", "}", 1},
		{TokenType::RIGHT_BRACE, "}", "}", 1},
		{TokenType::END_OF_FILE, "\n", "\n", 1}
	});
}

TEST_F(ParserTestFixture, ImportAfterForLoopEndsPassed) {
	// for (var i = 0; i < 1; i = i + 1) { } import "x" alias y;
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
		{TokenType::NUMBER, "1", 1.0, 1},
		{TokenType::SEMICOLON, ";", ";", 1},
		{TokenType::IDENTIFIER, "i", "i", 1},
		{TokenType::EQUAL, "=", "=", 1},
		{TokenType::IDENTIFIER, "i", "i", 1},
		{TokenType::PLUS, "+", "+", 1},
		{TokenType::NUMBER, "1", 1.0, 1},
		{TokenType::RIGHT_PAREN, ")", ")", 1},
		{TokenType::LEFT_BRACE, "{", "{", 1},
		{TokenType::RIGHT_BRACE, "}", "}", 1},
		{TokenType::IMPORT, "import", "import", 1},
		{TokenType::STRING, "\"x\"", "x", 1},
		{TokenType::ALIAS, "alias", "alias", 1},
		{TokenType::IDENTIFIER, "y", "y", 1},
		{TokenType::SEMICOLON, ";", ";", 1},
		{TokenType::END_OF_FILE, "\n", "\n", 1}
	});

	ASSERT_EQ(program.size(), 2);
	EXPECT_NE(dynamic_cast<const ForStmt*>(program[0].get()), nullptr);
	EXPECT_NE(dynamic_cast<const ImportStmt*>(program[1].get()), nullptr);
}
