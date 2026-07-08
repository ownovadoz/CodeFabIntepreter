#include "SemanticChecker.h"

#include "../AssemblerUnit/Parser/Parser.h"
#include "../AssemblerUnit/Parser/Expression.h"
#include "../AssemblerUnit/Tokenizer/Token.h"

#include <gmock/gmock.h>
#include <vector>

using std::vector;

TEST(SemanticCheckerTest, ParsedVarDeclareStmtWithoutErrorSucceeds) {
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

	SemanticChecker checker;

	EXPECT_FALSE(checker.check(root).hasError);
}

TEST(SemanticCheckerTest, DuplicateDeclarationInSameBlockFails) {
	BlockStmt* block = new BlockStmt();
	VarDeclareStmt* first = new VarDeclareStmt{ Token{TokenType::IDENTIFIER, "a", "a", 1} };
	first->setExpression(new LiteralExpr{ Token{TokenType::NUMBER, "1", 1.0, 1} });
	VarDeclareStmt* second = new VarDeclareStmt{ Token{TokenType::IDENTIFIER, "a", "a", 1} };
	second->setExpression(new LiteralExpr{ Token{TokenType::NUMBER, "2", 2.0, 1} });

	block->addStatement(first);
	block->addStatement(second);

	SemanticChecker checker;

	EXPECT_TRUE(checker.check(block).hasError);
}

TEST(SemanticCheckerTest, SameNameInNestedBlockSucceeds) {
	BlockStmt* inner = new BlockStmt();
	VarDeclareStmt* inner_var = new VarDeclareStmt{ Token{TokenType::IDENTIFIER, "a", "a", 1} };
	inner_var->setExpression(new LiteralExpr{ Token{TokenType::NUMBER, "2", 2.0, 1} });
	inner->addStatement(inner_var);

	BlockStmt* outer = new BlockStmt();
	VarDeclareStmt* outer_var = new VarDeclareStmt{ Token{TokenType::IDENTIFIER, "a", "a", 1} };
	outer_var->setExpression(new LiteralExpr{ Token{TokenType::NUMBER, "1", 1.0, 1} });
	outer->addStatement(outer_var);
	outer->addStatement(inner);

	SemanticChecker checker;

	EXPECT_FALSE(checker.check(outer).hasError);
}
