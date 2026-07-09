#include "FileModeShell.h"
#include "../CodeFabException.h"

#include <gmock/gmock.h>
#include <string>

using std::string;
using namespace testing;

TEST(FileModeShellTest, RunWithExistingFileDoesNotThrow) {
	FileModeShell shell([](const string&) { return true; });

	EXPECT_NO_THROW(shell.run("script.txt"));
}

TEST(FileModeShellTest, RunWithMissingFileThrowsCodeFabException) {
	FileModeShell shell([](const string&) { return false; });

	EXPECT_THROW(shell.run("missing.txt"), CodeFabException);
}
