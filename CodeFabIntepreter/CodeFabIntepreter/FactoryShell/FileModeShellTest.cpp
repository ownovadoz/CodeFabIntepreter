#ifdef _DEBUG

#include "FileModeShell.h"
#include "../CodeFabException.h"

#include <gmock/gmock.h>
#include <string>
#include <vector>

using std::string;
using std::vector;
using namespace testing;

TEST(FileModeShellTest, RunWithMissingFileThrowsCodeFabException) {
	FileModeShell shell([](const string&) { return false; });

	EXPECT_THROW(shell.run("missing.txt"), CodeFabException);
}

TEST(FileModeShellTest, RunWithNoLinesDoesNotThrow) {
	FileModeShell shell(
		[](const string&) { return true; },
		[](const string&) { return vector<string>{}; });

	EXPECT_NO_THROW(shell.run("script.txt"));
}

TEST(FileModeShellTest, RunExecutesEachLineAndTracksTheLastOne) {
	FileModeShell shell(
		[](const string&) { return true; },
		[](const string&) { return vector<string>{"var a = 1;", "var b = 2;"}; });

	shell.run("script.txt");

	EXPECT_EQ(shell.getLastLine(), "var b = 2;");
}

#endif
