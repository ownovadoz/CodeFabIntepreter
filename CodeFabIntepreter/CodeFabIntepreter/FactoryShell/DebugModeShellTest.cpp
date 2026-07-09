#ifdef _DEBUG

#include "DebugModeShell.h"
#include "../CodeFabException.h"

#include <gmock/gmock.h>
#include <string>
#include <vector>

using std::string;
using std::vector;
using namespace testing;

TEST(DebugModeShellTest, EnterWithMissingFileThrowsCodeFabException) {
	DebugModeShell shell("missing.txt", [](const string&) { return false; });

	EXPECT_THROW(shell.enter(), CodeFabException);
}

TEST(DebugModeShellTest, EnterWithExistingFileLoadsLines) {
	DebugModeShell shell(
		"script.txt",
		[](const string&) { return true; },
		[](const string&) { return vector<string>{"var a = 1;", "var b = 2;"}; });

	shell.enter();

	EXPECT_THAT(shell.getLoadedLines(), ElementsAre("var a = 1;", "var b = 2;"));
}

#endif
