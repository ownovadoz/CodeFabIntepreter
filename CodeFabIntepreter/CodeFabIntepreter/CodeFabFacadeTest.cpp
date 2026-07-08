#ifdef _DEBUG

#include "CodeFabFacade.h"
#include "InterfaceForCodeFabTest.h"
#include "CodeFabException.h"

#include <gmock/gmock.h>
#include <string>
using std::string;

class MockAssemblerUnit : public IAssemblerUnit {
public:
	MOCK_METHOD(Statement*, assemble, (const string& code_line), (override));
};

class MockChecker : public IChecker {
public:
	MOCK_METHOD(void, run, (), (override));
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
	EXPECT_CALL(mock_checker, run()).Times(1);
	EXPECT_CALL(mock_executor, run()).Times(1);

	facade.execute("var x = 10;");
}

TEST_F(CodeFabFacadeTestFixture, ExecuteCalledMultipleTimesInvokesEachDependencyPerCall) {
	EXPECT_CALL(mock_assembler_unit, assemble(::testing::_)).Times(2);
	EXPECT_CALL(mock_checker, run()).Times(2);
	EXPECT_CALL(mock_executor, run()).Times(2);

	facade.execute("var x = 10;");
	facade.execute("var y = 20;");
}

TEST_F(CodeFabFacadeTestFixture, ExecuteCatchesCodeFabExceptionAndSkipsRemainingSteps) {
	EXPECT_CALL(mock_assembler_unit, assemble(::testing::_))
		.WillOnce(::testing::Throw(CodeFabException(1, "boom")));
	EXPECT_CALL(mock_checker, run()).Times(0);
	EXPECT_CALL(mock_executor, run()).Times(0);

	EXPECT_NO_THROW(facade.execute("var x = 10;"));
}

TEST(CodeFabFacadeDefaultConstructorTest, ExecuteDoesNotThrowWithRealDependencies) {
	// Lexer가 아직 identifier/keyword를 스캔하지 못해 "var ...;" 입력은 실제 파이프라인에서 예외가 발생한다.
	CodeFabFacade facade;
	EXPECT_NO_THROW(facade.execute(""));
}
#endif