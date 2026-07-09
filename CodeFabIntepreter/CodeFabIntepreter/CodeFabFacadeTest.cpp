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
	MOCK_METHOD(void, run, (), (override));
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
	EXPECT_CALL(mock_executor, run()).Times(1);

	facade.execute("var x = 10;");
}

TEST_F(CodeFabFacadeTestFixture, ExecuteCalledMultipleTimesInvokesEachDependencyPerCall) {
	EXPECT_CALL(mock_assembler_unit, assemble(::testing::_)).Times(2);
	EXPECT_CALL(mock_checker, check(::testing::_)).Times(2);
	EXPECT_CALL(mock_executor, run()).Times(2);

	facade.execute("var x = 10;");
	facade.execute("var y = 20;");
}

TEST_F(CodeFabFacadeTestFixture, ExecuteCatchesCodeFabExceptionFromAssemblerUnitAndSkipsRemainingSteps) {
	EXPECT_CALL(mock_assembler_unit, assemble(::testing::_))
		.WillOnce(::testing::Throw(CodeFabException(1, "boom")));
	EXPECT_CALL(mock_checker, check(::testing::_)).Times(0);
	EXPECT_CALL(mock_executor, run()).Times(0);

	EXPECT_NO_THROW(facade.execute("var x = 10;"));
}

TEST_F(CodeFabFacadeTestFixture, ExecuteCatchesCodeFabExceptionFromCheckerAndSkipsRemainingSteps) {
	EXPECT_CALL(mock_assembler_unit, assemble(::testing::_)).Times(1);
	EXPECT_CALL(mock_checker, check(::testing::_))
		.WillOnce(::testing::Throw(CodeFabException(1, "boom")));
	EXPECT_CALL(mock_executor, run()).Times(0);

	EXPECT_NO_THROW(facade.execute("var x = 10;"));
}

TEST_F(CodeFabFacadeTestFixture, ExecuteCatchesCodeFabExceptionFromExecutor) {
	EXPECT_CALL(mock_assembler_unit, assemble(::testing::_)).Times(1);
	EXPECT_CALL(mock_checker, check(::testing::_)).Times(1);
	EXPECT_CALL(mock_executor, run())
		.WillOnce(::testing::Throw(CodeFabException(1, "boom")));

	EXPECT_NO_THROW(facade.execute("var x = 10;"));
}

TEST_F(CodeFabFacadeTestFixture, ExecuteCatchesStdExceptionFromAssemblerUnitAndSkipsRemainingSteps) {
	// CodeFabException 이외의 표준 라이브러리 예외(예: std::stod의 out_of_range)가
	// 새어나와도 REPL 프로세스가 죽지 않아야 한다.
	EXPECT_CALL(mock_assembler_unit, assemble(::testing::_))
		.WillOnce(::testing::Throw(std::out_of_range("boom")));
	EXPECT_CALL(mock_checker, check(::testing::_)).Times(0);
	EXPECT_CALL(mock_executor, run()).Times(0);

	EXPECT_NO_THROW(facade.execute("var x = 10;"));
}

TEST_F(CodeFabFacadeTestFixture, ExecuteCatchesUnknownExceptionFromAssemblerUnitAndSkipsRemainingSteps) {
	// std::exception 계층에 속하지 않는 임의의 값이 던져져도 방어되어야 한다.
	EXPECT_CALL(mock_assembler_unit, assemble(::testing::_))
		.WillOnce(::testing::Throw(42));
	EXPECT_CALL(mock_checker, check(::testing::_)).Times(0);
	EXPECT_CALL(mock_executor, run()).Times(0);

	EXPECT_NO_THROW(facade.execute("var x = 10;"));
}

TEST(CodeFabFacadeDefaultConstructorTest, ExecuteDoesNotThrowWithRealDependencies) {
	CodeFabFacade facade;
	EXPECT_NO_THROW(facade.execute(""));
}
#endif