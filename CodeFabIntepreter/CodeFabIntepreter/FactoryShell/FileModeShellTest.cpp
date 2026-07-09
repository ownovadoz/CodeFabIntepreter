#ifdef _DEBUG

#include "FileModeShell.h"
#include "../CodeFabException.h"

#include <gmock/gmock.h>
#include <string>
#include <vector>

using std::string;
using std::vector;
using namespace testing;

TEST(FileModeShellTest, EnterWithMissingFileThrowsCodeFabException) {
	FileModeShell shell("missing.txt", [](const string&) { return false; });

	EXPECT_THROW(shell.enter(), CodeFabException);
}

TEST(FileModeShellTest, EnterWithNoLinesDoesNotThrow) {
	FileModeShell shell(
		"script.txt",
		[](const string&) { return true; },
		[](const string&) { return vector<string>{}; });

	EXPECT_NO_THROW(shell.enter());
}

TEST(FileModeShellTest, EnterExecutesEachLineAndTracksTheLastOne) {
	FileModeShell shell(
		"script.txt",
		[](const string&) { return true; },
		[](const string&) { return vector<string>{"var a = 1;", "var b = 2;"}; });

	shell.enter();

	EXPECT_EQ(shell.getLastLine(), "var b = 2;");
}

TEST(FileModeShellTest, EnterStopsAtFirstRuntimeErrorAndDoesNotThrow) {
	// 2번째 줄에서 실제 CodeFabException이 발생하면, 3번째 줄은 처리되지 않고 즉시 멈춰야 한다.
	FileModeShell shell(
		"script.txt",
		[](const string&) { return true; },
		[](const string&) { return vector<string>{"var a = 1;", "var b = ;", "var c = 3;"}; });

	EXPECT_NO_THROW(shell.enter());
	EXPECT_EQ(shell.getLastLine(), "var b = ;");
}

#endif
