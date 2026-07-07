#include <gmock/gmock.h>
#include <iostream>
#include <string>

#include "PromptShell.h"

using std::string;

class PromptShellTestFixture : public ::testing::Test {
protected:
	void SetUp() override {
		originalCinBuffer = std::cin.rdbuf();
	}

	void TearDown() override {
		std::cin.rdbuf(originalCinBuffer);
	}
	string runPromptTest(const std::string& input) {
		feedInput(input);
		shell.runPrompt();
		return shell.getLine();
	}
	void feedInput(const std::string& input) {
		inputStream.str(input);
		inputStream.clear();
		std::cin.rdbuf(inputStream.rdbuf());
	}

private:
	std::istringstream inputStream;
	std::streambuf* originalCinBuffer = nullptr;
	PromptShell shell;
};

TEST_F(PromptShellTestFixture, NormalLineTest) {
	string input = "var x = 10;";
	EXPECT_EQ(input, runPromptTest(input));
}

TEST_F(PromptShellTestFixture, EmptyLineTest) {
	string input = "";
	EXPECT_EQ(input, runPromptTest(input));
}

TEST_F(PromptShellTestFixture, WithLineFeedTest) {
	string input = "var x = 10;\n var y = 20;\n var z = 30;";
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