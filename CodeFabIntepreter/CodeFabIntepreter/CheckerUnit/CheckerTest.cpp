#include "Checker.h"

#include "../AssemblerUnit/Parser/Parser.h"
#include "../AssemblerUnit/Parser/Expression.h"
#include "../AssemblerUnit/Tokenizer/Token.h"
#include "../CodeFabException.h"

#include <gmock/gmock.h>
#include <memory>
#include <optional>
#include <vector>

using std::make_unique;
using std::move;
using std::vector;

namespace {
	Token makeIdentifier(const string& name) {
		return Token(TokenType::IDENTIFIER, name, name, 1);
	}
}

class CheckerTest : public ::testing::Test {
protected:
    void SetUp() override { guard.emplace(checker); }

    Checker checker;
    std::optional<Checker::ScopeGuard> guard;
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
    Checker::ScopeGuard inner(checker);

    EXPECT_NO_THROW(checker.declareVariable(makeIdentifier("a"), {}));
}

TEST_F(CheckerTest, RedeclaringAfterScopeExitSucceeds) {
    {
        Checker::ScopeGuard inner(checker);
        checker.declareVariable(makeIdentifier("a"), {});
    }

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
	auto root = parser.parse(tokens);

	Checker checker;

	EXPECT_NO_THROW(checker.check(root.get()));
}

TEST(CheckerTreeTest, DuplicateDeclarationInSameBlockFails) {
	auto first = make_unique<VarDeclareStmt>(Token{TokenType::IDENTIFIER, "a", "a", 1});
	first->setExpression(make_unique<LiteralExpr>(Token{TokenType::NUMBER, "1", 1.0, 1}));

	auto second = make_unique<VarDeclareStmt>(Token{TokenType::IDENTIFIER, "a", "a", 1});
	second->setExpression(make_unique<LiteralExpr>(Token{TokenType::NUMBER, "2", 2.0, 1}));

	auto block = make_unique<BlockStmt>();
	block->addStatement(move(first));
	block->addStatement(move(second));

	Checker checker;

	EXPECT_THROW(checker.check(block.get()), CodeFabException);
}

TEST(CheckerTreeTest, SameNameInNestedBlockSucceeds) {
	auto inner_var = make_unique<VarDeclareStmt>(Token{TokenType::IDENTIFIER, "a", "a", 1});
	inner_var->setExpression(make_unique<LiteralExpr>(Token{TokenType::NUMBER, "2", 2.0, 1}));

	auto inner = make_unique<BlockStmt>();
	inner->addStatement(move(inner_var));

	auto outer_var = make_unique<VarDeclareStmt>(Token{TokenType::IDENTIFIER, "a", "a", 1});
	outer_var->setExpression(make_unique<LiteralExpr>(Token{TokenType::NUMBER, "1", 1.0, 1}));

	auto outer = make_unique<BlockStmt>();
	outer->addStatement(move(outer_var));
	outer->addStatement(move(inner));

	Checker checker;

	EXPECT_NO_THROW(checker.check(outer.get()));
}
