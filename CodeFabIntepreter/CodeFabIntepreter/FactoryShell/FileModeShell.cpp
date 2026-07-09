#include "FileModeShell.h"

#include <filesystem>
#include <fstream>
#include <utility>

using std::ifstream;
using std::move;

FileModeShell::FileModeShell(function<bool(const string&)> file_exists) : file_exists(move(file_exists)) {}

void FileModeShell::run(const string& file_path) {
	if (!file_exists(file_path)) throw CodeFabException(0, "파일을 찾을 수 없습니다: '" + file_path + "'");

	ifstream file(file_path);
	string input_line;
	while (std::getline(file, input_line)) {
		code_line = input_line;
		code_fab_facade.execute(code_line);
	}
}

bool FileModeShell::defaultFileExists(const string& path) {
	return std::filesystem::exists(path);
}
