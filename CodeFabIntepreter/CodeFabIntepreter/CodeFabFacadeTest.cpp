#ifdef _DEBUG

#include "CodeFabFacade.h"
#include "InterfaceForCodeFabTest.h"

#include <gmock/gmock.h>
#include <string>
using std::string;

class MockTokenizer : public ITokenizer {
public:
	MOCK_METHOD(void, run, (const string& code_line), (override));
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
	CodeFabFacadeTestFixture() : facade(mock_tokenizer, mock_checker, mock_executor) {}
	MockTokenizer mock_tokenizer;
	MockChecker mock_checker;
	MockExecutor mock_executor;
	CodeFabFacade facade;
};

TEST_F(CodeFabFacadeTestFixture, SequenceTest) {
	
	::testing::InSequence seq;
	EXPECT_CALL(mock_tokenizer, run(string("var x = 10;"))).Times(1);
	EXPECT_CALL(mock_checker, run()).Times(1);
	EXPECT_CALL(mock_executor, run()).Times(1);

	facade.execute("var x = 10;");
}

TEST_F(CodeFabFacadeTestFixture, ExecuteCalledMultipleTimesInvokesEachDependencyPerCall) {
	EXPECT_CALL(mock_tokenizer, run(::testing::_)).Times(2);
	EXPECT_CALL(mock_checker, run()).Times(2);
	EXPECT_CALL(mock_executor, run()).Times(2);

	facade.execute("var x = 10;");
	facade.execute("var y = 20;");
}

TEST(CodeFabFacadeDefaultConstructorTest, ExecuteDoesNotThrowWithRealDependencies) {
	CodeFabFacade facade;
	EXPECT_NO_THROW(facade.execute("var x = 10;"));
}
#endif