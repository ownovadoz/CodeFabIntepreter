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

TEST(DebugModeShellTest, EnterWithNoLinesDoesNotThrow) {
	DebugModeShell shell(
		"script.txt",
		[](const string&) { return true; },
		[](const string&) { return vector<string>{}; });

	EXPECT_NO_THROW(shell.enter());
}

TEST(DebugModeShellTest, EnterWithExistingFileLoadsLinesInOrder) {
	DebugModeShell shell(
		"script.txt",
		[](const string&) { return true; },
		[](const string&) { return vector<string>{"var a = 1;", "var b = 2;"}; });

	shell.enter();

	EXPECT_THAT(shell.getLoadedLines(), ElementsAre("var a = 1;", "var b = 2;"));
}

TEST(DebugModeShellTest, EnterWithNoLinesLoadsEmptyLines) {
	DebugModeShell shell(
		"script.txt",
		[](const string&) { return true; },
		[](const string&) { return vector<string>{}; });

	shell.enter();

	EXPECT_THAT(shell.getLoadedLines(), IsEmpty());
}

TEST(DebugModeShellTest, EnterPassesFilePathToFileExistsAndReadLines) {
	string received_exists_path;
	string received_read_path;
	DebugModeShell shell(
		"script.txt",
		[&received_exists_path](const string& path) { received_exists_path = path; return true; },
		[&received_read_path](const string& path) { received_read_path = path; return vector<string>{}; });

	shell.enter();

	EXPECT_EQ(received_exists_path, "script.txt");
	EXPECT_EQ(received_read_path, "script.txt");
}

#endif
