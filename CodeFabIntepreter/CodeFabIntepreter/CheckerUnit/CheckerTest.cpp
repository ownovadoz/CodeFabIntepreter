#include "Checker.h"

#include "../AssemblerUnit/Parser/Parser.h"
#include "../AssemblerUnit/Parser/Expression.h"
#include "../AssemblerUnit/Tokenizer/Token.h"
#include "../CodeFabException.h"

#include <gmock/gmock.h>
#include <vector>

using std::vector;

namespace {
	Token makeIdentifier(const string& name) {
		return Token(TokenType::IDENTIFIER, name, name, 1);
	}
}

class CheckerTest : public ::testing::Test {
protected:
    void SetUp() override { checker.enterScope(); }

    Checker checker;
};

TEST_F(CheckerTest, DeclaringNewVariableSucceeds) {
    EXPECT_NO_THROW(checker.declareVariable(makeIdentifier("a"), {}));
}

TEST_F(CheckerTest, DuplicateDeclarationInSameScopeFails) {
    checker.declareVariable(makeIdentifier("a"), {});

    EXPECT_THROW(checker.declareVariable(makeIdentifier("a"), {}), CodeFabException);
}

TEST_F(CheckerTest, SameNameInNestedScopeSucceeds) {
    checker.declareVariable(makeIdentifier("a"), {});
    checker.enterScope();

    EXPECT_NO_THROW(checker.declareVariable(makeIdentifier("a"), {}));
}

TEST_F(CheckerTest, RedeclaringAfterScopeExitSucceeds) {
    checker.enterScope();
    checker.declareVariable(makeIdentifier("a"), {});
    checker.exitScope();

    EXPECT_NO_THROW(checker.declareVariable(makeIdentifier("a"), {}));
}

TEST_F(CheckerTest, SelfReferenceInInitializerFails) {
    EXPECT_THROW(checker.declareVariable(makeIdentifier("a"), { "a" }), CodeFabException);
}

TEST_F(CheckerTest, ReferencingOtherVariableInInitializerSucceeds) {
    checker.declareVariable(makeIdentifier("b"), {});

    EXPECT_NO_THROW(checker.declareVariable(makeIdentifier("a"), { "b" }));
}

TEST(CheckerTreeTest, ParsedVarDeclareStmtWithoutErrorSucceeds) {
	vector<Token> tokens = {
		{TokenType::VAR, "var", "var", 1},
		{TokenType::IDENTIFIER, "a", "a", 1},
		{TokenType::EQUAL, "=", "=", 1},
		{TokenType::NUMBER, "10", 10.0, 1},
		{TokenType::SEMICOLON, ";", ";", 1},
		{TokenType::END_OF_FILE, "\n", "\n", 1}
	};

	Parser parser;
	Statement* root = parser.parse(tokens);

	Checker checker;

	EXPECT_NO_THROW(checker.check(root));

	delete root;
}

TEST(CheckerTreeTest, DuplicateDeclarationInSameBlockFails) {
	LiteralExpr* first_value = new LiteralExpr{ Token{TokenType::NUMBER, "1", 1.0, 1} };
	VarDeclareStmt* first = new VarDeclareStmt{ Token{TokenType::IDENTIFIER, "a", "a", 1} };
	first->setExpression(first_value);

	LiteralExpr* second_value = new LiteralExpr{ Token{TokenType::NUMBER, "2", 2.0, 1} };
	VarDeclareStmt* second = new VarDeclareStmt{ Token{TokenType::IDENTIFIER, "a", "a", 1} };
	second->setExpression(second_value);

	BlockStmt* block = new BlockStmt();
	block->addStatement(first);
	block->addStatement(second);

	Checker checker;

	EXPECT_THROW(checker.check(block), CodeFabException);

	delete block;
	delete first;
	delete second;
	delete first_value;
	delete second_value;
}

TEST(CheckerTreeTest, SameNameInNestedBlockSucceeds) {
	LiteralExpr* inner_value = new LiteralExpr{ Token{TokenType::NUMBER, "2", 2.0, 1} };
	VarDeclareStmt* inner_var = new VarDeclareStmt{ Token{TokenType::IDENTIFIER, "a", "a", 1} };
	inner_var->setExpression(inner_value);

	BlockStmt* inner = new BlockStmt();
	inner->addStatement(inner_var);

	LiteralExpr* outer_value = new LiteralExpr{ Token{TokenType::NUMBER, "1", 1.0, 1} };
	VarDeclareStmt* outer_var = new VarDeclareStmt{ Token{TokenType::IDENTIFIER, "a", "a", 1} };
	outer_var->setExpression(outer_value);

	BlockStmt* outer = new BlockStmt();
	outer->addStatement(outer_var);
	outer->addStatement(inner);

	Checker checker;

	EXPECT_NO_THROW(checker.check(outer));

	delete outer;
	delete inner;
	delete outer_var;
	delete inner_var;
	delete outer_value;
	delete inner_value;
}
