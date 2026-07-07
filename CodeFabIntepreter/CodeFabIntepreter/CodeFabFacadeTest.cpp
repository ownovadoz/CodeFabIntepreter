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

TEST(CodeFabFacadeTest, ExecuteCallsTokenizerThenCheckerThenExecutor) {
	MockTokenizer mock_tokenizer;
	MockChecker mock_checker;
	MockExecutor mock_executor;
	CodeFabFacade facade(mock_tokenizer, mock_checker, mock_executor);

	::testing::InSequence seq;
	EXPECT_CALL(mock_tokenizer, run(string("var x = 10;"))).Times(1);
	EXPECT_CALL(mock_checker, run()).Times(1);
	EXPECT_CALL(mock_executor, run()).Times(1);

	facade.execute("var x = 10;");
}
#endif