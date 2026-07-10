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

	vector<unique_ptr<Statement>> single(unique_ptr<Statement> statement) {
		vector<unique_ptr<Statement>> statements;
		statements.push_back(move(statement));

		return statements;
	}
}

class CheckerTestFixture : public testing::Test {
public:
	Checker checker;
};

TEST_F(CheckerTestFixture, ParsedVarDeclareStmtWithoutErrorSucceeds) {
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

	EXPECT_NO_THROW(checker.check(single(move(root))));
}

TEST_F(CheckerTestFixture, DuplicateDeclarationInSameBlockFails) {
	auto first = make_unique<VarDeclareStmt>(Token{TokenType::IDENTIFIER, "a", "a", 1});
	first->setExpression(make_unique<LiteralExpr>(Token{TokenType::NUMBER, "1", 1.0, 1}));

	auto second = make_unique<VarDeclareStmt>(Token{TokenType::IDENTIFIER, "a", "a", 1});
	second->setExpression(make_unique<LiteralExpr>(Token{TokenType::NUMBER, "2", 2.0, 1}));

	auto block = make_unique<BlockStmt>();
	block->addStatement(move(first));
	block->addStatement(move(second));

	EXPECT_THROW(checker.check(single(move(block))), CodeFabException);
}

TEST_F(CheckerTestFixture, SelfReferenceThroughBinaryExprInInitializerFails) {
	auto stmt = assemble("var a = a + 1;");

	EXPECT_THROW(checker.check(single(move(stmt))), CodeFabException);
}

TEST_F(CheckerTestFixture, SelfReferenceThroughUnaryExprInInitializerFails) {
	auto stmt = assemble("var a = -a;");

	EXPECT_THROW(checker.check(single(move(stmt))), CodeFabException);
}

TEST_F(CheckerTestFixture, SelfReferenceThroughGroupingExprInInitializerFails) {
	auto stmt = assemble("var a = (a);");

	EXPECT_THROW(checker.check(single(move(stmt))), CodeFabException);
}

TEST_F(CheckerTestFixture, SelfReferenceThroughLogicalExprInInitializerFails) {
	auto stmt = assemble("var a = a and true;");

	EXPECT_THROW(checker.check(single(move(stmt))), CodeFabException);
}

TEST_F(CheckerTestFixture, ReferencingOtherVariableThroughBinaryExprSucceeds) {
	auto stmt = assemble("var a = b + 1;");

	EXPECT_NO_THROW(checker.check(single(move(stmt))));
}

TEST_F(CheckerTestFixture, ReferencingAlreadyDefinedVariableInSameScopeSucceeds) {
	// 'a'는 같은 스코프에서 이미 정의(define)가 끝난 뒤이므로,
	// declare 상태(false)로 남아있는 'b' 자신과는 구분되어야 한다.
	auto stmt = assemble("{ var a = 1; var b = a + 1; }");

	EXPECT_NO_THROW(checker.check(single(move(stmt))));
}

TEST_F(CheckerTestFixture, LiteralInitializerWithoutReferenceSucceeds) {
	auto stmt = assemble("var a = 1 + 2;");

	EXPECT_NO_THROW(checker.check(single(move(stmt))));
}

TEST_F(CheckerTestFixture, DuplicateDeclarationInIfThenBranchBlockFails) {
	auto stmt = assemble("if (true) { var a = 1; var a = 2; }");

	EXPECT_THROW(checker.check(single(move(stmt))), CodeFabException);
}

TEST_F(CheckerTestFixture, DeclarationsInMutuallyExclusiveIfBranchesSucceed) {
	auto stmt = assemble("if (true) var a = 1; else var a = 2;");

	EXPECT_NO_THROW(checker.check(single(move(stmt))));
}

TEST_F(CheckerTestFixture, SelfReferenceInsideIfBranchFails) {
	auto stmt = assemble("if (true) { var a = 1; if (true) { var b = b + a; } }");

	EXPECT_THROW(checker.check(single(move(stmt))), CodeFabException);
}

TEST_F(CheckerTestFixture, PrintStmtWithUndeclaredVariableSucceeds) {
	// Checker는 미선언 변수 참조를 검사하지 않는다 (Executor의 책임).
	auto stmt = assemble("print a + 1;");

	EXPECT_NO_THROW(checker.check(single(move(stmt))));
}

TEST_F(CheckerTestFixture, ExpressionStmtIsTraversedWithoutError) {
	auto stmt = assemble("a = 1;");

	EXPECT_NO_THROW(checker.check(single(move(stmt))));
}

TEST_F(CheckerTestFixture, ForStmtAllowsShadowingLoopVariableInBody) {
	auto stmt = assemble("for (var i = 0; i < 3; i = i + 1) { var i = 1; }");

	EXPECT_NO_THROW(checker.check(single(move(stmt))));
}

TEST_F(CheckerTestFixture, ForStmtOwnScopeAllowsSeparateLoopsToReuseSameVariableName) {
	auto stmt = assemble(
		"{ for (var i = 0; i < 3; i = i + 1) { print i; } for (var i = 0; i < 3; i = i + 1) { print i; } }");

	EXPECT_NO_THROW(checker.check(single(move(stmt))));
}

TEST_F(CheckerTestFixture, ForStmtSelfReferenceInInitializerFails) {
	auto stmt = assemble("for (var i = i; i < 3; i = i + 1) { print i; }");

	EXPECT_THROW(checker.check(single(move(stmt))), CodeFabException);
}

TEST_F(CheckerTestFixture, ImportStmtInsideLoopBodyFails) {
	auto stmt = assemble("for (var i = 0; i < 3; i = i + 1) { import \"a.txt\" alias a; }");

	EXPECT_THROW(checker.check(single(move(stmt))), CodeFabException);
}

TEST_F(CheckerTestFixture, ImportStmtInsideFunctionDeclaredInsideLoopBodySucceeds) {
	// 함수 몸통은 그 함수가 반복문 안에서 선언됐더라도 호출될 때만 실행되므로,
	// 함수 경계를 넘으면 반복문 문맥이 리셋되어 import가 허용되어야 한다.
	auto stmt = assemble("for (var i = 0; i < 3; i = i + 1) { Func f() { import \"a.txt\" alias a; } }");

	EXPECT_NO_THROW(checker.check(single(move(stmt))));
}

TEST_F(CheckerTestFixture, ImportStmtAtTopLevelSucceeds) {
	auto stmt = assemble("import \"a.txt\" alias a;");

	EXPECT_NO_THROW(checker.check(single(move(stmt))));
}

TEST_F(CheckerTestFixture, DuplicateImportAliasInSameScopeFails) {
	AssemblerUnit assembler;
	auto statements = assembler.assemble("import \"a.txt\" alias a; import \"b.txt\" alias a;");

	ASSERT_EQ(statements.size(), 2u);

	EXPECT_THROW(checker.check(statements), CodeFabException);
}

TEST_F(CheckerTestFixture, ImportAliasCollidingWithExistingVariableNameFails) {
	AssemblerUnit assembler;
	auto statements = assembler.assemble("var a = 10; import \"a.txt\" alias a;");

	ASSERT_EQ(statements.size(), 2u);

	EXPECT_THROW(checker.check(statements), CodeFabException);
}

TEST_F(CheckerTestFixture, SeparateImportsWithDifferentAliasesInSameScopeSucceed) {
	AssemblerUnit assembler;
	auto statements = assembler.assemble("import \"a.txt\" alias a; import \"b.txt\" alias b;");

	ASSERT_EQ(statements.size(), 2u);

	EXPECT_NO_THROW(checker.check(statements));
}

TEST_F(CheckerTestFixture, DuplicateDeclarationAcrossSeparateTopLevelChecksFails) {
	// REPL에서 한 줄씩 입력되는 각 문장은 같은 Checker 인스턴스로 검사되므로,
	// 전역 스코프의 중복 선언은 호출이 나뉘어도 검출되어야 한다.
	auto first = assemble("var a = 10;");
	checker.check(single(move(first)));

	auto second = assemble("var a = 20;");

	EXPECT_THROW(checker.check(single(move(second))), CodeFabException);
}

TEST_F(CheckerTestFixture, DifferentVariablesAcrossSeparateTopLevelChecksSucceed) {
	auto first = assemble("var a = 10;");
	checker.check(single(move(first)));

	auto second = assemble("var b = 20;");

	EXPECT_NO_THROW(checker.check(single(move(second))));
}

TEST_F(CheckerTestFixture, ComplexNestedProgramWithoutErrorsSucceeds) {
	auto stmt = assemble(
		"{ var a = 1; if (a > 0) { var b = a + 1; print b; } "
		"for (var i = 0; i < a; i = i + 1) { print i; } }");

	EXPECT_NO_THROW(checker.check(single(move(stmt))));
}

TEST_F(CheckerTestFixture, ComplexNestedProgramWithDeepSelfReferenceFails) {
	auto stmt = assemble(
		"{ var a = 1; if (a > 0) { for (var i = 0; i < a; i = i + 1) { var b = b + i; } } }");

	EXPECT_THROW(checker.check(single(move(stmt))), CodeFabException);
}

TEST_F(CheckerTestFixture, AllStatementsInMultiStatementLineAreCheckedInOrder) {
	// var a = 3; a = a + 4; { var b = 3; } print b;
	AssemblerUnit assembler;
	auto statements = assembler.assemble("var a = 3; a = a + 4; { var b = 3; } print b;");

	ASSERT_EQ(statements.size(), 4u);

	EXPECT_NO_THROW(checker.check(statements));
}

TEST_F(CheckerTestFixture, DuplicateDeclarationWithinSameMultiStatementLineFails) {
	// 같은 줄에 세미콜론으로 이어진 여러 문장이라도, 전역 스코프의
	// 중복 선언은 문장이 나뉘어도 순서대로 검사하면 검출되어야 한다.
	AssemblerUnit assembler;
	auto statements = assembler.assemble("var a = 3; var a = 4;");

	ASSERT_EQ(statements.size(), 2u);

	EXPECT_THROW(checker.check(statements), CodeFabException);
}

TEST_F(CheckerTestFixture, VarDeclareStmtWithoutInitializerSucceeds) {
	// Parser 문법상 var 선언은 항상 '='을 요구해 초기화식이 없는 VarDeclareStmt는
	// 실제 파이프라인으로 만들어지지 않지만, Checker 자체는 initializer가
	// nullptr인 경우(resolveExpr의 null 가드)도 안전하게 처리해야 한다.
	auto stmt = make_unique<VarDeclareStmt>(Token{TokenType::IDENTIFIER, "a", "a", 1});

	EXPECT_NO_THROW(checker.check(single(move(stmt))));
}

TEST_F(CheckerTestFixture, EmptyBlockSucceeds) {
	auto block = make_unique<BlockStmt>();

	EXPECT_NO_THROW(checker.check(single(move(block))));
}

TEST_F(CheckerTestFixture, SameNameInNestedBlockSucceeds) {
	auto inner_var = make_unique<VarDeclareStmt>(Token{TokenType::IDENTIFIER, "a", "a", 1});
	inner_var->setExpression(make_unique<LiteralExpr>(Token{TokenType::NUMBER, "2", 2.0, 1}));

	auto inner = make_unique<BlockStmt>();
	inner->addStatement(move(inner_var));

	auto outer_var = make_unique<VarDeclareStmt>(Token{TokenType::IDENTIFIER, "a", "a", 1});
	outer_var->setExpression(make_unique<LiteralExpr>(Token{TokenType::NUMBER, "1", 1.0, 1}));

	auto outer = make_unique<BlockStmt>();
	outer->addStatement(move(outer_var));
	outer->addStatement(move(inner));

	EXPECT_NO_THROW(checker.check(single(move(outer))));
}

TEST_F(CheckerTestFixture, FunctionDeclarationWithoutErrorSucceeds) {
	auto stmt = assemble("Func add(a, b) { return a + b; }");

	EXPECT_NO_THROW(checker.check(single(move(stmt))));
}

TEST_F(CheckerTestFixture, RecursiveFunctionCallSucceeds) {
	// 함수는 var와 달리 자신의 몸통 안에서 스스로를 참조(재귀 호출)할 수 있어야 한다.
	auto stmt = assemble("Func fact(n) { if (n <= 1) return 1; return n * fact(n - 1); }");

	EXPECT_NO_THROW(checker.check(single(move(stmt))));
}

TEST_F(CheckerTestFixture, DuplicateParameterNameFails) {
	auto stmt = assemble("Func foo(a, a) { return a; }");

	EXPECT_THROW(checker.check(single(move(stmt))), CodeFabException);
}

TEST_F(CheckerTestFixture, DuplicateFunctionDeclarationInSameScopeFails) {
	AssemblerUnit assembler;
	auto statements = assembler.assemble("Func add(a, b) { return a + b; } Func add(a) { return a; }");

	ASSERT_EQ(statements.size(), 2u);

	EXPECT_THROW(checker.check(statements), CodeFabException);
}

TEST_F(CheckerTestFixture, ReturnStmtOutsideFunctionFails) {
	auto stmt = assemble("return 5;");

	EXPECT_THROW(checker.check(single(move(stmt))), CodeFabException);
}

TEST_F(CheckerTestFixture, ReturnStmtInsideNestedBlockOfFunctionSucceeds) {
	auto stmt = assemble("Func foo() { { return 1; } }");

	EXPECT_NO_THROW(checker.check(single(move(stmt))));
}

TEST_F(CheckerTestFixture, ReturnStmtWithoutValueInsideFunctionSucceeds) {
	auto stmt = assemble("Func foo() { return; }");

	EXPECT_NO_THROW(checker.check(single(move(stmt))));
}

TEST_F(CheckerTestFixture, ReturnStmtAfterFunctionEndsFails) {
	// 함수 몸통을 벗어난 뒤의 return은 함수 depth가 정상적으로 복원되어 오류가 나야 한다.
	AssemblerUnit assembler;
	auto statements = assembler.assemble("Func foo() { return 1; } return 2;");

	ASSERT_EQ(statements.size(), 2u);

	EXPECT_THROW(checker.check(statements), CodeFabException);
}

TEST_F(CheckerTestFixture, ParametersAreVisibleInsideFunctionBody) {
	auto stmt = assemble("Func identity(a) { return a; }");

	EXPECT_NO_THROW(checker.check(single(move(stmt))));
}

TEST_F(CheckerTestFixture, FunctionParametersDoNotLeakToOuterScope) {
	AssemblerUnit assembler;
	auto statements = assembler.assemble("Func identity(a) { return a; } var a = 10;");

	ASSERT_EQ(statements.size(), 2u);

	EXPECT_NO_THROW(checker.check(statements));
}

TEST_F(CheckerTestFixture, CallExprResolvesCalleeAndArguments) {
	auto stmt = assemble("add(1, 2);");

	EXPECT_NO_THROW(checker.check(single(move(stmt))));
}

TEST_F(CheckerTestFixture, CallExprWithSelfReferencingArgumentInInitializerFails) {
	auto stmt = assemble("var a = add(a);");

	EXPECT_THROW(checker.check(single(move(stmt))), CodeFabException);
}

TEST_F(CheckerTestFixture, NullStatementEntryInVectorIsSkipped) {
	// AssemblerUnit이 실제로 nullptr을 담아 반환하는 일은 없지만, check()는
	// vector 안에 nullptr이 섞여 있어도 안전하게 건너뛰어야 한다.
	vector<unique_ptr<Statement>> statements;
	statements.push_back(nullptr);
	statements.push_back(assemble("var a = 10;"));
	statements.push_back(nullptr);

	EXPECT_NO_THROW(checker.check(statements));
}

TEST_F(CheckerTestFixture, ClassDeclarationWithMethodsSucceeds) {
	auto stmt = assemble("Class Robot { move(dist) { this.position = this.position + dist; } }");

	EXPECT_NO_THROW(checker.check(single(move(stmt))));
}

TEST_F(CheckerTestFixture, InitializerSucceedsWithoutReturn) {
	auto stmt = assemble("Class Robot { init(name) { this.name = name; } }");

	EXPECT_NO_THROW(checker.check(single(move(stmt))));
}

TEST_F(CheckerTestFixture, ReturnInsideInitializerFails) {
	auto stmt = assemble("Class Robot { init() { return 5; } }");

	EXPECT_THROW(checker.check(single(move(stmt))), CodeFabException);
}

TEST_F(CheckerTestFixture, ThisOutsideClassFails) {
	auto stmt = assemble("print this;");

	EXPECT_THROW(checker.check(single(move(stmt))), CodeFabException);
}

TEST_F(CheckerTestFixture, SuperOutsideClassFails) {
	auto stmt = assemble("Super.move();");

	EXPECT_THROW(checker.check(single(move(stmt))), CodeFabException);
}

TEST_F(CheckerTestFixture, SuperInClassWithoutSuperclassFails) {
	auto stmt = assemble("Class Robot { move() { Super.move(); } }");

	EXPECT_THROW(checker.check(single(move(stmt))), CodeFabException);
}

TEST_F(CheckerTestFixture, SuperInClassWithSuperclassSucceeds) {
	vector<unique_ptr<Statement>> statements;
	statements.push_back(assemble("Class Robot { move() { } }"));
	statements.push_back(assemble("Class SpeedRobot : Robot { move() { Super.move(); } }"));

	EXPECT_NO_THROW(checker.check(statements));
}

TEST_F(CheckerTestFixture, SelfInheritanceFails) {
	auto stmt = assemble("Class Robot : Robot { }");

	EXPECT_THROW(checker.check(single(move(stmt))), CodeFabException);
}

TEST_F(CheckerTestFixture, ArrayCreationAndIndexReadWriteSucceeds) {
	auto stmt = assemble("var arr = Array(3);");

	EXPECT_NO_THROW(checker.check(single(move(stmt))));
}

TEST_F(CheckerTestFixture, ArraySizeSelfReferenceInInitializerFails) {
	auto stmt = assemble("var a = Array(a);");

	EXPECT_THROW(checker.check(single(move(stmt))), CodeFabException);
}

TEST_F(CheckerTestFixture, IndexExprWithSelfReferencingArrayInInitializerFails) {
	auto stmt = assemble("var a = a[0];");

	EXPECT_THROW(checker.check(single(move(stmt))), CodeFabException);
}

TEST_F(CheckerTestFixture, IndexSetExprWithSelfReferencingValueInInitializerFails) {
	auto stmt = assemble("var a = (arr[0] = a);");

	EXPECT_THROW(checker.check(single(move(stmt))), CodeFabException);
}
