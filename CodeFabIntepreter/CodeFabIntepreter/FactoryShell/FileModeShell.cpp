#include "FileModeShell.h"

#include <filesystem>
#include <fstream>

#ifdef _DEBUG
#include <utility>
using std::move;
#endif

using std::ifstream;

#ifdef _DEBUG

FileModeShell::FileModeShell(function<bool(const string&)> file_exists, function<vector<string>(const string&)> read_lines)
	: file_exists(move(file_exists)), read_lines(move(read_lines)) {}

void FileModeShell::run(const string& file_path) {
	if (!file_exists(file_path)) throw CodeFabException(0, "파일을 찾을 수 없습니다: '" + file_path + "'");

	for (const string& input_line : read_lines(file_path)) {
		code_line = input_line;
		code_fab_facade.execute(code_line);
	}
}

#else

void FileModeShell::run(const string& file_path) {
	if (!defaultFileExists(file_path)) throw CodeFabException(0, "파일을 찾을 수 없습니다: '" + file_path + "'");

	for (const string& input_line : defaultReadLines(file_path)) {
		code_line = input_line;
		code_fab_facade.execute(code_line);
	}
}

#endif

bool FileModeShell::defaultFileExists(const string& path) {
	return std::filesystem::exists(path);
}

vector<string> FileModeShell::defaultReadLines(const string& path) {
	vector<string> lines;
	ifstream file(path);
	string line;
	while (std::getline(file, line)) {
		lines.push_back(line);
	}
	return lines;
}
