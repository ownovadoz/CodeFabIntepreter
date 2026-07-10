#ifdef _DEBUG

#include "FileBackedShell.h"
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
	// FileBackedShell(file_path)만 넘기면 file_exists/read_lines 인자가 기본값
	// (defaultFileExists/defaultReadLines)으로 채워져 실제 디스크를 읽는다. 이
	// 기본 구현 자체를 검증하려면 진짜 임시 파일이 있어야 한다.
	class TempFile {
	public:
		explicit TempFile(const string& content) {
			path = (std::filesystem::temp_directory_path() / "codefab_filebackedshell_test.txt").string();
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

	string captureStdout(FileBackedShell& shell) {
		ostringstream captured;
		std::streambuf* original_buf = std::cout.rdbuf(captured.rdbuf());
		shell.enter();
		std::cout.rdbuf(original_buf);
		return captured.str();
	}
}

TEST(FileBackedShellTest, EnterWithRealFileOnDiskReadsLinesAndExecutesThem) {
	TempFile file("var a = 1;\nprint a;\n");
	FileBackedShell shell(file.getPath());

	EXPECT_EQ(captureStdout(shell), "1\n");
}

TEST(FileBackedShellTest, EnterWithRealMissingFileOnDiskThrowsCodeFabException) {
	FileBackedShell shell((std::filesystem::temp_directory_path() / "codefab_filebackedshell_missing.txt").string());

	EXPECT_THROW(shell.enter(), CodeFabException);
}

#endif
