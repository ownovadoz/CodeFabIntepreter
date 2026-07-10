#ifdef _DEBUG

#include "FileModeShell.h"
#include "../CodeFabException.h"

#include <gmock/gmock.h>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

using std::ofstream;
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

namespace {
	// FileModeShell(file_path)만 넘기면 file_exists/read_source 인자가 기본값
	// (FileBackedShell::defaultFileExists / FileModeShell::defaultReadSource)으로
	// 채워져 실제 디스크를 읽는다. 이 두 기본 구현 자체(가짜 파일 시스템을 굳이
	// 주입하지 않는 실제 경로)를 검증하려면 진짜 임시 파일이 있어야 한다.
	class TempFile {
	public:
		TempFile(const string& content) {
			path = (std::filesystem::temp_directory_path() / "codefab_filemode_test.txt").string();
			ofstream file(path, std::ios::binary);
			file << content;
		}

		~TempFile() {
			std::filesystem::remove(path);
		}

		const string& getPath() const { return path; }

	private:
		string path;
	};
}

TEST(FileModeShellTest, EnterWithRealFileOnDiskReadsAndExecutesItsActualContent) {
	// 마지막 줄에 개행이 없는 파일도 정상적으로 한 줄로 읽혀야 한다
	// (FileModeShell의 splitIntoLines가 마지막 조각을 개행 없이 처리하는 경로).
	TempFile file("print 1 + 2;");
	FileModeShell shell(file.getPath());

	EXPECT_EQ(captureStdout(shell), "3\n");
}

TEST(FileModeShellTest, EnterWithRealMissingFileOnDiskThrowsCodeFabException) {
	FileModeShell shell((std::filesystem::temp_directory_path() / "codefab_filemode_test_missing.txt").string());

	EXPECT_THROW(shell.enter(), CodeFabException);
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
