#ifdef _DEBUG

#include "FileModeShell.h"
#include "../CodeFabException.h"

#include <gmock/gmock.h>
#include <iostream>
#include <sstream>
#include <string>

using std::ostringstream;
using std::string;
using namespace testing;

namespace {
	string captureStdout(FileModeShell& shell) {
		ostringstream captured;
		std::streambuf* original_buf = std::cout.rdbuf(captured.rdbuf());
		shell.enter();
		std::cout.rdbuf(original_buf);
		return captured.str();
	}

	string captureStderr(FileModeShell& shell) {
		ostringstream captured;
		std::streambuf* original_buf = std::cerr.rdbuf(captured.rdbuf());
		shell.enter();
		std::cerr.rdbuf(original_buf);
		return captured.str();
	}
}

TEST(FileModeShellTest, EnterWithMissingFileThrowsCodeFabException) {
	FileModeShell shell("missing.txt", [](const string&) { return false; });

	EXPECT_THROW(shell.enter(), CodeFabException);
}

TEST(FileModeShellTest, EnterWithEmptySourceDoesNotThrow) {
	FileModeShell shell(
		"script.txt",
		[](const string&) { return true; },
		[](const string&) { return string(); });

	EXPECT_NO_THROW(shell.enter());
}

TEST(FileModeShellTest, EnterExecutesStatementsSpanningMultiplePhysicalLines) {
	// 블록처럼 여러 줄에 걸친 문장도 파일 전체를 하나의 소스로 파싱/실행해야 한다.
	FileModeShell shell(
		"script.txt",
		[](const string&) { return true; },
		[](const string&) {
			return string(
				"var x = \"global\";\n"
				"{\n"
				"  var x = \"inner\";\n"
				"  print x;\n"
				"}\n"
				"print x;\n");
		});

	EXPECT_EQ(captureStdout(shell), "inner\nglobal\n");
}

TEST(FileModeShellTest, EnterOnRuntimeErrorDoesNotThrow) {
	FileModeShell shell(
		"script.txt",
		[](const string&) { return true; },
		[](const string&) { return string("print notDefined;\n"); });

	EXPECT_NO_THROW(shell.enter());
}

TEST(FileModeShellTest, EnterPrintsRuntimeErrorMessageToStderr) {
	FileModeShell shell(
		"script.txt",
		[](const string&) { return true; },
		[](const string&) { return string("print notDefined;\n"); });

	EXPECT_THAT(captureStderr(shell), HasSubstr("Undefined variable 'notDefined'"));
}

#endif
