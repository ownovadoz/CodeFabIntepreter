#include "FileModeShell.h"
#include "../CodeFabException.h"

#include <gmock/gmock.h>
#include <filesystem>
#include <fstream>
#include <string>

using std::ofstream;
using std::string;
using namespace testing;

namespace {
	class TempScriptFile {
	public:
		explicit TempScriptFile(const string& content) : path("file_mode_shell_test_temp.txt") {
			ofstream file(path);
			file << content;
		}
		~TempScriptFile() {
			std::filesystem::remove(path);
		}
		const string& getPath() const { return path; }

	private:
		string path;
	};
}

TEST(FileModeShellTest, RunWithMissingFileThrowsCodeFabException) {
	FileModeShell shell([](const string&) { return false; });

	EXPECT_THROW(shell.run("missing.txt"), CodeFabException);
}

TEST(FileModeShellTest, RunWithEmptyFileDoesNotThrow) {
	TempScriptFile temp_file("");
	FileModeShell shell;

	EXPECT_NO_THROW(shell.run(temp_file.getPath()));
}

TEST(FileModeShellTest, RunExecutesEachLineAndTracksTheLastOne) {
	TempScriptFile temp_file("var a = 1;\nvar b = 2;\n");
	FileModeShell shell;

	shell.run(temp_file.getPath());

	EXPECT_EQ(shell.getLastLine(), "var b = 2;");
}
