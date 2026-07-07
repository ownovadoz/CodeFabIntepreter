#include "PromptShell.h"

#include <gmock/gmock.h>
#include <iostream>
#include <string>
using std::string;

TEST(PromptShellTest, NormalLineTest) {
	// Test the basic functionality of the PromptShell class
	std::istringstream inputStream;
	std::streambuf* originalCinBuffer = nullptr;
	originalCinBuffer = std::cin.rdbuf();
	string input = "var x = 10;";
	inputStream.str(input);
	inputStream.clear();
	std::cin.rdbuf(inputStream.rdbuf());

	PromptShell shell;
	shell.runPrompt();

	EXPECT_EQ(input, shell.getLine());
	std::cin.rdbuf(originalCinBuffer);
}

TEST(PromptShellTest, EmptyLineTest) {
	// Test the basic functionality of the PromptShell class
	std::istringstream inputStream;
	std::streambuf* originalCinBuffer = nullptr;
	originalCinBuffer = std::cin.rdbuf();
	string input = "";
	inputStream.str(input);
	inputStream.clear();
	std::cin.rdbuf(inputStream.rdbuf());

	PromptShell shell;
	shell.runPrompt();

	EXPECT_EQ(input, shell.getLine());
	std::cin.rdbuf(originalCinBuffer);
}