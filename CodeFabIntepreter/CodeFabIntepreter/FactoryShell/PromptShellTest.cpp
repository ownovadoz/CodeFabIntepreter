#include <gmock/gmock.h>
#include <iostream>
#include <string>

#include "PromptShell.h"

using std::string;

class PromptShellTestFixture : public ::testing::Test {
protected:
	void SetUp() override {
		original_cin_buffer = std::cin.rdbuf();
	}

	void TearDown() override {
		std::cin.rdbuf(original_cin_buffer);
	}
	string runPromptTest(const std::string& input) {
		feedInput(input);
		shell.enter();
		return shell.getLine();
	}
	void feedInput(const std::string& input) {
		input_stream.str(input);
		input_stream.clear();
		std::cin.rdbuf(input_stream.rdbuf());
	}

private:
	std::istringstream input_stream;
	std::streambuf* original_cin_buffer = nullptr;
	PromptShell shell;
};

TEST_F(PromptShellTestFixture, NormalLineTest) {
	string input = "var a = \"11111111111\";";
	EXPECT_EQ(input, runPromptTest(input));
}

TEST_F(PromptShellTestFixture, EmptyLineTest) {
	string input = "";
	EXPECT_EQ(input, runPromptTest(input));
}

TEST_F(PromptShellTestFixture, MultilineBlockAccumulatesUntilBraceCloses) {
	string input = "{\n  var x = 1;\n}";
	string expected = "{\n  var x = 1;\n}";
	EXPECT_EQ(expected, runPromptTest(input));
}

TEST_F(PromptShellTestFixture, MultilineBlockWithEscapedNewline) {
	string input = "{\\n  var x = 1;\\n}";
	string expected = "{\n  var x = 1;\n}";
	EXPECT_EQ(expected, runPromptTest(input));
}

TEST_F(PromptShellTestFixture, ContinuesAfterRuntimeErrorTest) {
	// 첫 줄이 런타임 오류를 던져도 REPL은 종료되지 않고 다음 줄을 계속 처리해야 한다.
	string input = "var a = ;\nvar b = 2;";
	EXPECT_EQ("var b = 2;", runPromptTest(input));
}

TEST_F(PromptShellTestFixture, ExitTest) {
	string input = "exit";
	string expectedOutput = "";
	EXPECT_EQ(expectedOutput, runPromptTest(input));
}

TEST_F(PromptShellTestFixture, QuitTest) {
	string input = "quit";
	string expectedOutput = "";
	EXPECT_EQ(expectedOutput, runPromptTest(input));
}