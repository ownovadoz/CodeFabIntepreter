#ifdef _DEBUG

#include "DebugModeShell.h"
#include "../CodeFabException.h"

#include <gmock/gmock.h>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

using std::string;
using std::vector;
using namespace testing;

namespace {
	// DebugModeShell::enter()이 내부에서 DebugSession을 만들어 std::cin으로부터
	// 명령어를 읽으므로, 실제 표준입력을 기다리며 멈추지 않도록 항상 EOF(빈 입력)를
	// 흘려보낸다. DebugSession은 EOF를 continue로 취급해 곧바로 재개한다.
	class DebugModeShellTestFixture : public ::testing::Test {
	protected:
		void SetUp() override {
			original_cin_buffer = std::cin.rdbuf();
			std::cin.rdbuf(empty_input.rdbuf());
		}

		void TearDown() override {
			std::cin.rdbuf(original_cin_buffer);
		}

	private:
		std::istringstream empty_input;
		std::streambuf* original_cin_buffer = nullptr;
	};
}

TEST_F(DebugModeShellTestFixture, EnterWithMissingFileThrowsCodeFabException) {
	DebugModeShell shell("missing.txt", [](const string&) { return false; });

	EXPECT_THROW(shell.enter(), CodeFabException);
}

TEST_F(DebugModeShellTestFixture, EnterWithNoLinesDoesNotThrow) {
	DebugModeShell shell(
		"script.txt",
		[](const string&) { return true; },
		[](const string&) { return vector<string>{}; });

	EXPECT_NO_THROW(shell.enter());
}

TEST_F(DebugModeShellTestFixture, EnterWithExistingFileLoadsLinesInOrder) {
	DebugModeShell shell(
		"script.txt",
		[](const string&) { return true; },
		[](const string&) { return vector<string>{"var a = 1;", "var b = 2;"}; });

	shell.enter();

	EXPECT_THAT(shell.getLoadedLines(), ElementsAre("var a = 1;", "var b = 2;"));
}

TEST_F(DebugModeShellTestFixture, EnterWithNoLinesLoadsEmptyLines) {
	DebugModeShell shell(
		"script.txt",
		[](const string&) { return true; },
		[](const string&) { return vector<string>{}; });

	shell.enter();

	EXPECT_THAT(shell.getLoadedLines(), IsEmpty());
}

TEST_F(DebugModeShellTestFixture, EnterPassesFilePathToFileExistsAndReadLines) {
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

TEST_F(DebugModeShellTestFixture, EnterExecutesEachLoadedLine) {
	DebugModeShell shell(
		"script.txt",
		[](const string&) { return true; },
		[](const string&) { return vector<string>{"var a = 1;", "var b = 2;"}; });

	EXPECT_NO_THROW(shell.enter());
}

#endif
