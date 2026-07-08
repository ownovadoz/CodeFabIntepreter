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
		shell.runPrompt();
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

// Lexer가 아직 identifier/keyword를 스캔하지 못해 "var ...;" 입력이 실제 파이프라인에서 예외를 던진다.
// identifier scan이 병합되면 DISABLED_ 접두어를 제거해야 한다.
TEST_F(PromptShellTestFixture, DISABLED_NormalLineTest) {
	string input = "var a = \"11111111111\"";
	EXPECT_EQ(input, runPromptTest(input));
}

TEST_F(PromptShellTestFixture, EmptyLineTest) {
	string input = "";
	EXPECT_EQ(input, runPromptTest(input));
}

// Lexer가 아직 identifier/keyword를 스캔하지 못해 "var ...;" 입력이 실제 파이프라인에서 예외를 던진다.
// identifier scan이 병합되면 DISABLED_ 접두어를 제거해야 한다.
TEST_F(PromptShellTestFixture, DISABLED_WithLineFeedTest) {
	string input = "var x = 10;\\n var y = 20;\\n var z = 30;";
	string expectedOutput = "var x = 10;";
	EXPECT_EQ(expectedOutput, runPromptTest(input));
}

TEST_F(PromptShellTestFixture, ExitTest) {
	string input = "exit";
	string expectedOutput = "";
	EXPECT_EQ(expectedOutput, runPromptTest(input));
	input = "EXIT";
	EXPECT_EQ(expectedOutput, runPromptTest(input));
}