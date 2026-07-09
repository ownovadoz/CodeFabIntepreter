#include "Checker.h"

#include "../AssemblerUnit/AssemblerUnit.h"
#include "../AssemblerUnit/Parser/Parser.h"
#include "../AssemblerUnit/Parser/Expression.h"
#include "../AssemblerUnit/Tokenizer/Token.h"
#include "../CodeFabException.h"

#include <gmock/gmock.h>
#include <memory>
#include <vector>

using std::make_unique;
using std::move;
using std::vector;

namespace {
	unique_ptr<Statement> assemble(const string& source) {
		AssemblerUnit assembler;
		vector<unique_ptr<Statement>> statements = assembler.assemble(source);

		return statements.empty() ? nullptr : move(statements[0]);
	}
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
	auto root = move(parser.parse(tokens)[0]);

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

TEST(CheckerTreeTest, SelfReferenceThroughBinaryExprInInitializerFails) {
	auto stmt = assemble("var a = a + 1;");

	Checker checker;

	EXPECT_THROW(checker.check(stmt.get()), CodeFabException);
}

TEST(CheckerTreeTest, SelfReferenceThroughUnaryExprInInitializerFails) {
	auto stmt = assemble("var a = -a;");

	Checker checker;

	EXPECT_THROW(checker.check(stmt.get()), CodeFabException);
}

TEST(CheckerTreeTest, SelfReferenceThroughGroupingExprInInitializerFails) {
	auto stmt = assemble("var a = (a);");

	Checker checker;

	EXPECT_THROW(checker.check(stmt.get()), CodeFabException);
}

TEST(CheckerTreeTest, SelfReferenceThroughLogicalExprInInitializerFails) {
	auto stmt = assemble("var a = a and true;");

	Checker checker;

	EXPECT_THROW(checker.check(stmt.get()), CodeFabException);
}

TEST(CheckerTreeTest, ReferencingOtherVariableThroughBinaryExprSucceeds) {
	auto stmt = assemble("var a = b + 1;");

	Checker checker;

	EXPECT_NO_THROW(checker.check(stmt.get()));
}

TEST(CheckerTreeTest, ReferencingAlreadyDefinedVariableInSameScopeSucceeds) {
	// 'a'는 같은 스코프에서 이미 정의(define)가 끝난 뒤이므로,
	// declare 상태(false)로 남아있는 'b' 자신과는 구분되어야 한다.
	auto stmt = assemble("{ var a = 1; var b = a + 1; }");

	Checker checker;

	EXPECT_NO_THROW(checker.check(stmt.get()));
}

TEST(CheckerTreeTest, LiteralInitializerWithoutReferenceSucceeds) {
	auto stmt = assemble("var a = 1 + 2;");

	Checker checker;

	EXPECT_NO_THROW(checker.check(stmt.get()));
}

TEST(CheckerTreeTest, DuplicateDeclarationInIfThenBranchBlockFails) {
	auto stmt = assemble("if (true) { var a = 1; var a = 2; }");

	Checker checker;

	EXPECT_THROW(checker.check(stmt.get()), CodeFabException);
}

TEST(CheckerTreeTest, DeclarationsInMutuallyExclusiveIfBranchesSucceed) {
	auto stmt = assemble("if (true) var a = 1; else var a = 2;");

	Checker checker;

	EXPECT_NO_THROW(checker.check(stmt.get()));
}

TEST(CheckerTreeTest, SelfReferenceInsideIfBranchFails) {
	auto stmt = assemble("if (true) { var a = 1; if (true) { var b = b + a; } }");

	Checker checker;

	EXPECT_THROW(checker.check(stmt.get()), CodeFabException);
}

TEST(CheckerTreeTest, PrintStmtWithUndeclaredVariableSucceeds) {
	// Checker는 미선언 변수 참조를 검사하지 않는다 (Executor의 책임).
	auto stmt = assemble("print a + 1;");

	Checker checker;

	EXPECT_NO_THROW(checker.check(stmt.get()));
}

TEST(CheckerTreeTest, ExpressionStmtIsTraversedWithoutError) {
	auto stmt = assemble("a = 1;");

	Checker checker;

	EXPECT_NO_THROW(checker.check(stmt.get()));
}

TEST(CheckerTreeTest, ForStmtAllowsShadowingLoopVariableInBody) {
	auto stmt = assemble("for (var i = 0; i < 3; i = i + 1) { var i = 1; }");

	Checker checker;

	EXPECT_NO_THROW(checker.check(stmt.get()));
}

TEST(CheckerTreeTest, ForStmtOwnScopeAllowsSeparateLoopsToReuseSameVariableName) {
	auto stmt = assemble(
		"{ for (var i = 0; i < 3; i = i + 1) { print i; } for (var i = 0; i < 3; i = i + 1) { print i; } }");

	Checker checker;

	EXPECT_NO_THROW(checker.check(stmt.get()));
}

TEST(CheckerTreeTest, ForStmtSelfReferenceInInitializerFails) {
	auto stmt = assemble("for (var i = i; i < 3; i = i + 1) { print i; }");

	Checker checker;

	EXPECT_THROW(checker.check(stmt.get()), CodeFabException);
}

TEST(CheckerTreeTest, DuplicateDeclarationAcrossSeparateTopLevelChecksFails) {
	// REPL에서 한 줄씩 입력되는 각 문장은 같은 Checker 인스턴스로 검사되므로,
	// 전역 스코프의 중복 선언은 호출이 나뉘어도 검출되어야 한다.
	Checker checker;

	auto first = assemble("var a = 10;");
	checker.check(first.get());

	auto second = assemble("var a = 20;");

	EXPECT_THROW(checker.check(second.get()), CodeFabException);
}

TEST(CheckerTreeTest, DifferentVariablesAcrossSeparateTopLevelChecksSucceed) {
	Checker checker;

	auto first = assemble("var a = 10;");
	checker.check(first.get());

	auto second = assemble("var b = 20;");

	EXPECT_NO_THROW(checker.check(second.get()));
}

TEST(CheckerTreeTest, ComplexNestedProgramWithoutErrorsSucceeds) {
	auto stmt = assemble(
		"{ var a = 1; if (a > 0) { var b = a + 1; print b; } "
		"for (var i = 0; i < a; i = i + 1) { print i; } }");

	Checker checker;

	EXPECT_NO_THROW(checker.check(stmt.get()));
}

TEST(CheckerTreeTest, ComplexNestedProgramWithDeepSelfReferenceFails) {
	auto stmt = assemble(
		"{ var a = 1; if (a > 0) { for (var i = 0; i < a; i = i + 1) { var b = b + i; } } }");

	Checker checker;

	EXPECT_THROW(checker.check(stmt.get()), CodeFabException);
}

TEST(CheckerTreeTest, AllStatementsInMultiStatementLineAreCheckedInOrder) {
	// var a = 3; a = a + 4; { var b = 3; } print b;
	AssemblerUnit assembler;
	auto statements = assembler.assemble("var a = 3; a = a + 4; { var b = 3; } print b;");

	ASSERT_EQ(statements.size(), 4u);

	Checker checker;
	for (const auto& statement : statements)
		EXPECT_NO_THROW(checker.check(statement.get()));
}

TEST(CheckerTreeTest, DuplicateDeclarationWithinSameMultiStatementLineFails) {
	// 같은 줄에 세미콜론으로 이어진 여러 문장이라도, 전역 스코프의
	// 중복 선언은 문장이 나뉘어도 순서대로 검사하면 검출되어야 한다.
	AssemblerUnit assembler;
	auto statements = assembler.assemble("var a = 3; var a = 4;");

	ASSERT_EQ(statements.size(), 2u);

	Checker checker;
	checker.check(statements[0].get());

	EXPECT_THROW(checker.check(statements[1].get()), CodeFabException);
}

TEST(CheckerTreeTest, VarDeclareStmtWithoutInitializerSucceeds) {
	// Parser 문법상 var 선언은 항상 '='을 요구해 초기화식이 없는 VarDeclareStmt는
	// 실제 파이프라인으로 만들어지지 않지만, Checker 자체는 initializer가
	// nullptr인 경우(resolveExpr의 null 가드)도 안전하게 처리해야 한다.
	auto stmt = make_unique<VarDeclareStmt>(Token{TokenType::IDENTIFIER, "a", "a", 1});

	Checker checker;

	EXPECT_NO_THROW(checker.check(stmt.get()));
}

TEST(CheckerTreeTest, EmptyBlockSucceeds) {
	auto block = make_unique<BlockStmt>();

	Checker checker;

	EXPECT_NO_THROW(checker.check(block.get()));
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
