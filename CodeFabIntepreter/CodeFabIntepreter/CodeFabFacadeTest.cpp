#ifdef _DEBUG

#include "CodeFabFacade.h"
#include "InterfaceForCodeFabTest.h"
#include "AssemblerUnit/Parser/Statement.h"
#include "CodeFabException.h"

#include <gmock/gmock.h>
#include <stdexcept>
#include <string>
using std::string;

class MockAssemblerUnit : public IAssemblerUnit {
public:
	MOCK_METHOD(unique_ptr<Statement>, assemble, (const string& code_line), (override));
};

class MockChecker : public IChecker {
public:
	MOCK_METHOD(void, check, (Statement* root), (override));
};

class MockExecutor : public IExecutor {
public:
	MOCK_METHOD(void, interpret, (Statement* root), (override));
};

class CodeFabFacadeTestFixture : public ::testing::Test {
public:
	CodeFabFacadeTestFixture() : facade(mock_assembler_unit, mock_checker, mock_executor) {}
	MockAssemblerUnit mock_assembler_unit;
	MockChecker mock_checker;
	MockExecutor mock_executor;
	CodeFabFacade facade;
};

TEST_F(CodeFabFacadeTestFixture, SequenceTest) {

	::testing::InSequence seq;
	EXPECT_CALL(mock_assembler_unit, assemble(string("var x = 10;"))).Times(1);
	EXPECT_CALL(mock_checker, check(::testing::_)).Times(1);
	EXPECT_CALL(mock_executor, interpret(::testing::_)).Times(1);

	facade.execute("var x = 10;");
}

TEST_F(CodeFabFacadeTestFixture, ExecuteCalledMultipleTimesInvokesEachDependencyPerCall) {
	EXPECT_CALL(mock_assembler_unit, assemble(::testing::_)).Times(2);
	EXPECT_CALL(mock_checker, check(::testing::_)).Times(2);
	EXPECT_CALL(mock_executor, interpret(::testing::_)).Times(2);

	facade.execute("var x = 10;");
	facade.execute("var y = 20;");
}

TEST_F(CodeFabFacadeTestFixture, ExecuteCatchesCodeFabExceptionFromAssemblerUnitAndSkipsRemainingSteps) {
	EXPECT_CALL(mock_assembler_unit, assemble(::testing::_))
		.WillOnce(::testing::Throw(CodeFabException(1, "boom")));
	EXPECT_CALL(mock_checker, check(::testing::_)).Times(0);
	EXPECT_CALL(mock_executor, interpret(::testing::_)).Times(0);

	EXPECT_NO_THROW(facade.execute("var x = 10;"));
}

TEST_F(CodeFabFacadeTestFixture, ExecuteCatchesCodeFabExceptionFromCheckerAndSkipsRemainingSteps) {
	EXPECT_CALL(mock_assembler_unit, assemble(::testing::_)).Times(1);
	EXPECT_CALL(mock_checker, check(::testing::_))
		.WillOnce(::testing::Throw(CodeFabException(1, "boom")));
	EXPECT_CALL(mock_executor, interpret(::testing::_)).Times(0);

	EXPECT_NO_THROW(facade.execute("var x = 10;"));
}

TEST_F(CodeFabFacadeTestFixture, ExecuteCatchesCodeFabExceptionFromExecutor) {
	EXPECT_CALL(mock_assembler_unit, assemble(::testing::_)).Times(1);
	EXPECT_CALL(mock_checker, check(::testing::_)).Times(1);
	EXPECT_CALL(mock_executor, interpret(::testing::_))
		.WillOnce(::testing::Throw(CodeFabException(1, "boom")));

	EXPECT_NO_THROW(facade.execute("var x = 10;"));
}

TEST_F(CodeFabFacadeTestFixture, ExecuteCatchesStdExceptionFromAssemblerUnitAndSkipsRemainingSteps) {
	// CodeFabException 이외의 표준 라이브러리 예외(예: std::stod의 out_of_range)가
	// 새어나와도 REPL 프로세스가 죽지 않아야 한다.
	EXPECT_CALL(mock_assembler_unit, assemble(::testing::_))
		.WillOnce(::testing::Throw(std::out_of_range("boom")));
	EXPECT_CALL(mock_checker, check(::testing::_)).Times(0);
	EXPECT_CALL(mock_executor, interpret(::testing::_)).Times(0);

	EXPECT_NO_THROW(facade.execute("var x = 10;"));
}

TEST_F(CodeFabFacadeTestFixture, ExecuteCatchesUnknownExceptionFromAssemblerUnitAndSkipsRemainingSteps) {
	// std::exception 계층에 속하지 않는 임의의 값이 던져져도 방어되어야 한다.
	EXPECT_CALL(mock_assembler_unit, assemble(::testing::_))
		.WillOnce(::testing::Throw(42));
	EXPECT_CALL(mock_checker, check(::testing::_)).Times(0);
	EXPECT_CALL(mock_executor, interpret(::testing::_)).Times(0);

	EXPECT_NO_THROW(facade.execute("var x = 10;"));
}

TEST(CodeFabFacadeDefaultConstructorTest, ExecuteDoesNotThrowWithRealDependencies) {
	CodeFabFacade facade;
	EXPECT_NO_THROW(facade.execute(""));
}

TEST(CodeFabFacadeDefaultConstructorTest, ExecuteCatchesRealCodeFabExceptionFromInvalidSyntax) {
	// "var a = ;" 는 실제 Parser가 초기화식 누락으로 CodeFabException을 던지는 입력이다.
	CodeFabFacade facade;
	EXPECT_NO_THROW(facade.execute("var a = ;"));
}

TEST(CodeFabFacadeDefaultConstructorTest, ExecuteCatchesRealCheckerSelfReferenceError) {
	// "var a = a + 1;" 은 실제 Checker가 초기화식 자기참조로 CodeFabException을 던지는 입력이다.
	CodeFabFacade facade;
	EXPECT_NO_THROW(facade.execute("var a = a + 1;"));
}

TEST(CodeFabFacadeDefaultConstructorTest, ExecuteCatchesRealCheckerDuplicateDeclarationAcrossLines) {
	// 같은 Facade 인스턴스로 여러 줄을 실행하는 REPL 시나리오에서, 전역 스코프의
	// 중복 선언은 줄이 나뉘어도 실제 Checker에 의해 검출되어야 한다.
	CodeFabFacade facade;
	facade.execute("var a = 10;");

	EXPECT_NO_THROW(facade.execute("var a = 20;"));
}
#endif