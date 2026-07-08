#include "Checker.h"

#include "../AssemblerUnit/Parser/Parser.h"
#include "../AssemblerUnit/Parser/Expression.h"
#include "../AssemblerUnit/Tokenizer/Token.h"

#include <gmock/gmock.h>
#include <vector>

using std::vector;

class CheckerTest : public ::testing::Test {
protected:
    void SetUp() override { checker.enterScope(); }

    Checker checker;
};

TEST_F(CheckerTest, DeclaringNewVariableSucceeds) {
    EXPECT_FALSE(checker.declareVariable("a", {}).hasError());
}

TEST_F(CheckerTest, DuplicateDeclarationInSameScopeFails) {
    checker.declareVariable("a", {});

    EXPECT_TRUE(checker.declareVariable("a", {}).hasError());
}

TEST_F(CheckerTest, SameNameInNestedScopeSucceeds) {
    checker.declareVariable("a", {});
    checker.enterScope();

    EXPECT_FALSE(checker.declareVariable("a", {}).hasError());
}

TEST_F(CheckerTest, RedeclaringAfterScopeExitSucceeds) {
    checker.enterScope();
    checker.declareVariable("a", {});
    checker.exitScope();

    EXPECT_FALSE(checker.declareVariable("a", {}).hasError());
}

TEST_F(CheckerTest, SelfReferenceInInitializerFails) {
    EXPECT_TRUE(checker.declareVariable("a", { "a" }).hasError());
}

TEST_F(CheckerTest, ReferencingOtherVariableInInitializerSucceeds) {
    checker.declareVariable("b", {});

    EXPECT_FALSE(checker.declareVariable("a", { "b" }).hasError());
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

	EXPECT_FALSE(checker.check(root).hasError());
}

TEST(CheckerTreeTest, DuplicateDeclarationInSameBlockFails) {
	BlockStmt* block = new BlockStmt();
	VarDeclareStmt* first = new VarDeclareStmt{ Token{TokenType::IDENTIFIER, "a", "a", 1} };
	first->setExpression(new LiteralExpr{ Token{TokenType::NUMBER, "1", 1.0, 1} });
	VarDeclareStmt* second = new VarDeclareStmt{ Token{TokenType::IDENTIFIER, "a", "a", 1} };
	second->setExpression(new LiteralExpr{ Token{TokenType::NUMBER, "2", 2.0, 1} });

	block->addStatement(first);
	block->addStatement(second);

	Checker checker;

	EXPECT_TRUE(checker.check(block).hasError());
}

TEST(CheckerTreeTest, SameNameInNestedBlockSucceeds) {
	BlockStmt* inner = new BlockStmt();
	VarDeclareStmt* inner_var = new VarDeclareStmt{ Token{TokenType::IDENTIFIER, "a", "a", 1} };
	inner_var->setExpression(new LiteralExpr{ Token{TokenType::NUMBER, "2", 2.0, 1} });
	inner->addStatement(inner_var);

	BlockStmt* outer = new BlockStmt();
	VarDeclareStmt* outer_var = new VarDeclareStmt{ Token{TokenType::IDENTIFIER, "a", "a", 1} };
	outer_var->setExpression(new LiteralExpr{ Token{TokenType::NUMBER, "1", 1.0, 1} });
	outer->addStatement(outer_var);
	outer->addStatement(inner);

	Checker checker;

	EXPECT_FALSE(checker.check(outer).hasError());
}
