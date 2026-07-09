#include "FileModeShell.h"

#include <filesystem>
#include <fstream>
#include <utility>

using std::ifstream;
using std::move;

#ifdef _DEBUG

FileModeShell::FileModeShell(string file_path, function<bool(const string&)> file_exists, function<vector<string>(const string&)> read_lines)
	: file_path(move(file_path)), file_exists(move(file_exists)), read_lines(move(read_lines)) {}

void FileModeShell::enter() {
	if (!file_exists(file_path)) throw CodeFabException(0, "파일을 찾을 수 없습니다: '" + file_path + "'");

	for (const string& input_line : read_lines(file_path)) {
		code_line = input_line;
		code_fab_facade.execute(code_line);
	}
}

#else

FileModeShell::FileModeShell(string file_path) : file_path(move(file_path)) {}

void FileModeShell::enter() {
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
